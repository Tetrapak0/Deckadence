#include "../../include/Header.hpp"
#include "../../include/GUI/GUI.hpp"
#include "../../include/Config.hpp"
#include <memory>

namespace NxSh {

void Profile::set_root(GUI::Folder* root) {
    this->m_root = root;
    if (root) this->items = &root->items;
    else      this->items = &this->m_items;
}

void Profile::set_editor_root(GUI::Folder* root) {
    this->m_editor_root = root;
    if (root) this->editor_items = &root->items;
    else      this->editor_items = &this->m_items;
}

mutex shells_lock;

unordered_map<string, Shell> shells = {};

/*  WIN32
    /// Numerous environment variables with paths that may be needed later.
    /// '%'s can be ommitted in getenv();
    %USERPROFILE% - user root dir
    %HOMEPATH% - user root dir without drive letter
    %HOMEDRIVE% - drive letter with user dir
    %APPDATA% - user/appdata/roaming
    %LOCALAPPDATA% - user/appdata/local
    %TEMP% / %TMP% - temp dir (%LOCALAPPDATA%/Temp)
    %USERNAME% - user login name
    ///
*/
/*  UNIX
    /// Numerous environment variables with paths that may be needed later.
    /// '$'s can be ommitted in getenv();
    $HOME - user's home dir
    $USER - user's login name
    $PWD  - current working dir
    $LOGNAME - same as $USER
    $TMP / $TMPDIR - temporary folder (/tmp)
    ///
*/
#ifdef _WIN32
 #define CFG_ENV_VAR "APPDATA"
 #define CFG_MISSING "NexusShell" / ""
#else
 #define CFG_ENV_VAR "HOME"
 #define CFG_MISSING ".config" / "NexusShell" / ""
#endif

// TODO: Class application, contains EVERYTHING, makes reloading MUCH easier
//       Also makes setting const variables easier

std::filesystem::path get_cfg_path() {
    using std::filesystem::path;
    return path(getenv(CFG_ENV_VAR)) / CFG_MISSING;
}

void config_read(Shell& shell) {
    if (shell.config[shell.config.begin().key()].contains("nickname"))
        shell.nickname = shell.config[shell.config.begin().key()]["nickname"].get<string>();
    if (shell.config[shell.config.begin().key()].contains("profiles")) {
        shell.profiles.clear();
        int profile_count = 0;
        for (auto& profile : shell.config[shell.config.begin().key()]["profiles"]) {
            if (profile.is_object()) ++profile_count;
        }
        if (!profile_count) {
            Profile profile;
            for (int a = 0; a < profile.size; ++a)
                profile.items->emplace_back(GUI::FileButton());
            shell.profiles.emplace_back(profile);
            return;
        }
        for (int i = 0; i < profile_count; ++i) {
            Profile profile;
            json& profiles = shell.config[shell.config.begin().key()]["profiles"][to_string(i)];
            if (profiles.contains("columns")) 
                profile.columns = std::stoi(profiles["columns"].get<string>());
            if (profiles.contains("rows")) 
                profile.rows = std::stoi(profiles["rows"].get<string>());
            profile.size = profile.columns * profile.rows;
            if (profiles.contains("pages")) {
                int page_count = 0;
                for (auto& page : profiles["pages"]) if (page.is_object()) page_count++;
                if (!page_count) {
                    for (int j = 0; j < profile.size; ++j) 
                        profile.items->emplace_back(GUI::FileButton());
                    continue;
                }
                for (int j = 0; j < page_count; ++j) {
                    json& pages = profiles["pages"][to_string(j)];
                    if (pages.contains("buttons")) {
                        for (int k = 0; k < profile.size; ++k) {
                            GUI::FileButton button;
                            if (pages["buttons"].contains(to_string(k))) {
                                json& buttons = pages["buttons"][to_string(k)];
                                if (buttons.contains("label")) button.set_label(buttons["label"]);
                                if (buttons.contains("display type")) {
                                    try {
                                        int type = std::stoi(buttons["display type"].get<string>());
                                        if (!type)
                                            button.uses_thumbnail = 0;
                                        else
                                            button.uses_thumbnail = 1;
                                    } catch (...) {}
                                }
                                button.label_bak = button.label;
                                if (buttons.contains("has default label")) {
                                    if (buttons["has default label"] == "1") button.default_label = true;
                                    else if (buttons["has default label"] == "0") button.default_label = false;
                                    else if ((button.label == "")) button.default_label = true;
                                    else button.default_label = false;
                                }
                                if (buttons.contains("type")) {
                                    try {
                                        int type = std::stoi(buttons["type"].get<string>());
                                        // TODO:
                                    } catch (...) {}
                                }
                                if (buttons.contains("action")) button.set_member("Path", buttons["action"]);
                            }
                            profile.items->emplace_back(button);
                        }
                    } else for (int k = 0; k < profile.size; ++k) profile.items->emplace_back(GUI::FileButton());
                }
            } else for (int j = 0; j < profile.size; ++j) profile.items->emplace_back(GUI::FileButton());
            shell.profiles.push_back(profile);
        }
    } else {
        Profile profile;
        for (int i = 0; i < profile.size; ++i)
            profile.items->emplace_back(GUI::FileButton());
        shell.profiles.push_back(profile);
    }
}

int configure_shell(Shell& shell) {
    std::filesystem::path cfg_path = get_cfg_path();
    // TODO: Switch to the client way of file & dir creation
    if (!exists(cfg_path)) create_directories(cfg_path);
    cfg_path /= (shell.get_uuid() + ".json");
    printf("%s\n", cfg_path.c_str());
    if (exists(cfg_path)) {
        ifstream reader(cfg_path);
        if (reader.peek() != ifstream::traits_type::eof()) {
            try {
                json config = json::parse(reader);
                reader.close();
                shell.config = config;
                config_read(shell);
                reconfigure_shell(shell);
                if (!shell.nickname.empty()) printf("Configured %s\n", shell.nickname.c_str());
                else printf("Configured %s\n", shell.get_uuid().c_str());
                return 0;
            } catch (...) {
                fprintf(stderr, "Invalid configuration for %s\n", shell.get_uuid().c_str());
                send(shell.client, "killreason: invalid config", strlen("killreason: invalid config") + 1, 0);
                nx_sock_close(shell.client);
                return 1;
            }
        }
        reader.close();
    }
    Profile profile;
    for (int i = 0; i < profile.size; ++i)
        profile.items->emplace_back(GUI::FileButton());
    shell.profiles.push_back(profile);
    string out = "{\"" + shell.get_uuid() + "\": {\"profiles\": {\"0\": {\"pages\": {\"0\": {\"buttons\": {}}}}}}}";
    ofstream writer(cfg_path);
    writer << out;
    writer.close();
    shell.config = json::parse(out);
    reconfigure_shell(shell);
    return 0;
}

void reconfigure_shell(Shell& shell) {
    json config = shell.config;
    // TODO: If type == 1, set label to "<image>" (profile/page/button.format) - the client will know what to do
    if (config[config.begin().key()].contains("nickname")) config[config.begin().key()].erase("nickname");
    for (size_t i = 0; i < shell.profiles.size(); ++i) {
        json profiles = config[config.begin().key()]["profiles"][to_string(i)];
        if (profiles.contains("pages")) {
            int page_count = 0;
            for (auto& page : profiles["pages"]) if (page.is_object()) ++page_count;
            if (!page_count) continue;
            for (int j = 0; j < page_count; ++j) {
                json pages = profiles["pages"][to_string(j)];
                if (pages.contains("buttons")) {
                    for (int k = 0; k < shell.profiles[i].columns * shell.profiles[i].rows; ++k) {
                        if (pages["buttons"].contains(to_string(k))) {
                            json buttons = pages["buttons"][to_string(k)];
                            if (buttons.contains("display type"))
                                if (buttons["display type"] == "1")
                                    if (buttons.contains("label"))
                                        buttons["label"] = "<image>";
                            if (buttons.contains("type")) buttons.erase("type");
                            if (buttons.contains("action"))
                                if (!buttons["action"].empty()) buttons["action"] = "1";
                        }
                    }
                }
            }
        }
        string out_cfg = "cfg" + config.dump();
        shell.send_res = send(shell.client, out_cfg.c_str(), out_cfg.length(), 0);
    }
}

void clear_button(GUI::Item& btn, int profile, int page, int button) {
    Shell& shell = shells[g_shell_uuid];
    string nxsh_config = string(getenv("APPDATA")) + "\\NexusShell\\";
    if (!exists(nxsh_config)) create_directories(nxsh_config);
    nxsh_config += "\\" + g_shell_uuid + ".json";
    if (shell.config[g_shell_uuid]["profiles"][to_string(profile)]["pages"][to_string(page)]["buttons"].contains(to_string(button))) {
        if (btn.uses_thumbnail) 
            btn.thumbnail.destroy();
        shell.config[g_shell_uuid]["profiles"][to_string(profile)]["pages"][to_string(page)]["buttons"].erase(to_string(button));
        ofstream writer(nxsh_config);
        writer << shell.config.dump(4);
        writer.close();
        config_read(shell);
    }
    printf("Cleared %d/%d/%d\n", profile, page, button);
}

void config_write(Shell& shell) {
    using std::filesystem::path;
    path nxsh_config = path(getenv(CFG_ENV_VAR)) / "NexusShell" / "";
    if (!exists(nxsh_config)) create_directories(nxsh_config);
    nxsh_config /= (shell.get_uuid() + ".json");
    ofstream writer(nxsh_config);
    writer << shell.config.dump(4);
    writer.close();
}

}