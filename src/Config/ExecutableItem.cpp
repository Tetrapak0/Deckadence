#include "Config/Deckastore.hpp"
#include "Config/Config.hpp"
#include "Config/Item.hpp"
#include "Config/ExecutableItem.hpp"
#include "Config/FolderItem.hpp"
#include "GUI/GUI.hpp"
#include "Utilities/FileDialog.hpp"

ExecutableItem::ExecutableItem(json* config, Profile& parent_profile, FolderItem* parent) : Item(config, parent_profile, parent) {
    if (config) {
        if (config->contains("command") && (*config)["command"].is_string()) {
            this->command = (*config)["command"].get<string>();
            this->m_command = command;
        }
        if (config->contains("args") && (*config)["args"].is_string()) {
            this->args = (*config)["args"].get<string>();
            this->m_args = args;
        }
        if (config->contains("cwd") && (*config)["cwd"].is_string()) {
            this->cwd = (*config)["cwd"].get<string>();
            this->m_cwd = cwd;
        }
        if (config->contains("admin") && (*config)["admin"].is_boolean()) {
            this->admin = (*config)["admin"].get<bool>();
            this->m_admin = admin;
        }
        if (config->contains("console") && (*config)["console"].is_boolean()) {
            this->console = (*config)["console"].get<bool>();
            this->m_console = console;
        }
        if (Deckastore::get().get_mode() == Deckadence::mode_t::Server)
            args_to_argv();
    }
}

void ExecutableItem::draw_properties() {
    ImGui::BeginDisabled();
    ImGui::TextWrapped("(i) Please do not use environment variables. Write your full user path (if applicable), not ~/");
    ImGui::EndDisabled();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Path: ");
    ImGui::SameLine();
    ImGui::InputText("##cmd", &this->m_command);
    ImGui::SameLine();
    if (ImGui::Button("Browse")) {
        string res = nfd_open_exe();
        if (!res.empty())
            m_command = res;
    }
    ImGui::BeginDisabled();
#ifdef _WIN32
    ImGui::TextWrapped("(i) If you cannot see some shortcuts, try looking at the public desktop or select the \"Filesystem / URL\" type in the \"General\" tab.");
#else
    ImGui::TextWrapped("(i) If you cannot see some shortcuts, try selecting the \"Filesystem / URL\" type in the \"General\" tab.");
#endif
    ImGui::EndDisabled();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Arguments: ");
    ImGui::SameLine();
    ImGui::InputText("##args", &this->m_args);
    ImGui::BeginDisabled();
    ImGui::TextWrapped("(i) Arguments that contain spaces (i.e. paths) should be put in double quotes");
    ImGui::EndDisabled();
    // fucking wayland
#ifdef _WIN32
    ImGui::Checkbox("Run with administrator rights", &this->m_admin);
#endif
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Working directory: ");
    ImGui::SameLine();
    ImGui::InputText("##cwd", &this->m_cwd);
    ImGui::SameLine();
    ImGui::PushID(69);
    if (ImGui::Button("Browse")) {
        string res = nfd_open_dir();
        if (!res.empty())
            m_cwd = res;
    }
    ImGui::PopID();
    ImGui::BeginDisabled();
    ImGui::TextWrapped("(i) If empty, Deckadence's current working directory will be used.");
    ImGui::EndDisabled();
#ifndef _WIN32
    ImGui::Checkbox("Show console", &this->m_console);
    // TODO: terminal selection
    // ImGui::BeginDisabled();
    // ImGui::TextWrapped("(i) This will use the first good terminal if one is not specified below:");
    // ImGui::EndDisabled();
#endif
}

bool ExecutableItem::disabled() {
    return this->m_command.empty();
}

void ExecutableItem::properties_cancel() {
    this->m_command = this->command;
    this->m_args = this->args;
    this->m_cwd = this->cwd;
    this->m_admin = this->admin;
    this->m_console = this->console;
}

void ExecutableItem::properties_apply() {
    this->command = this->m_command;
    this->args = this->m_args;
    this->cwd = this->m_cwd;
    this->admin = this->m_admin;
    this->console = this->m_console;
    (*this->config)["command"] = this->command;
    (*this->config)["args"] = this->args;
    (*this->config)["cwd"] = this->cwd;
    (*this->config)["admin"] = this->admin;
    (*this->config)["console"] = this->console;
    this->args_to_argv();
}

void ExecutableItem::args_to_argv() {
#ifndef _WIN32
    this->argv.clear();
    this->strargv.clear();

    string current;
    bool in_sig_quote = false;
    bool in_double_quote = false;
    bool backslash = false;

    this->argv.push_back(this->command.data());
    for (size_t i = 0; i < this->args.length(); ++i) {
        const char c = args[i];

        if (backslash) {
            current += c;
            backslash = false;
        } else if (c == '\\') {
            backslash = true;
        } else if (c == '"' && !in_sig_quote && !backslash) {
            in_double_quote = !in_double_quote;
        } else if (c == '\'' && !in_double_quote && !backslash) {
            in_sig_quote = !in_sig_quote;
        } else if (std::isspace(c) && !in_sig_quote && !in_double_quote && !backslash) {
            if (!current.empty()) {
                this->strargv.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        this->strargv.push_back(current);
    }

    for (auto& arg : strargv) {
        argv.push_back(arg.data());
    }
#endif
}

void ExecutableItem::execute() {
#ifdef _WIN32
    // TODO: Add window show options (show, minimise, etc.)
    // TODO: Windows store apps
    if (this->admin)
        ShellExecuteA(nullptr, "runas", this->command.c_str(), this->args.c_str(), this->cwd.empty() ? nullptr : this->cwd.c_str(), SW_SHOW);
    else
        ShellExecuteA(nullptr, "open", this->command.c_str(), this->args.c_str(), this->cwd.empty() ? nullptr : this->cwd.c_str(), SW_SHOW);
#else
    pid_t pid = fork();
    if (pid) {
        return;
    }

    setsid();

    int fd = open("/dev/null", O_RDWR);
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    if (fd > 2)
        close(fd);

    vector<char*> _argv;
    _argv.reserve(this->argv.size() + 4);
    if (this->console) {
        vector<string> terminals = {
            std::getenv("TERMINAL") ? std::getenv("TERMINAL") : "",
            "x-terminal-emulator", "mate-terminal", "gnome-terminal", "terminator",
            "xfce4-terminal", "urxvt", "rxvt", "termit", "Eterm", "aterm",
            "uxterm", "xterm", "roxterm", "termite", "lxterminal", "terminology",
            "st", "qterminal", "lilyterm", "tilix", "terminix", "konsole",
            "kitty", "guake", "tilda", "alacritty", "hyper", "wezterm", "rio"
        };

        for (const auto& terminal : terminals) {
            if (terminal.empty())
                continue;

            if (!system(string("command -v " + terminal + " > /dev/null 2>&1").c_str())) {
                _argv.push_back(vector<char*>::value_type(terminal.data()));
                _argv.push_back("-e");
                break;
            }
        }
    }
    // TODO: SUDO
    // we don't need administrative consoles
    // if (this->admin) {
    //     if (command == this->command)
    //         command = "xdg-su";
    //     _argv.push_back("xdg-su");
    // }
    _argv.insert(_argv.end(), this->argv.begin(), this->argv.end());
    _argv.push_back(nullptr);
    // for (auto& arg : _argv)
    //     printf("%s\n", arg);

    if (!this->cwd.empty())
        chdir(this->cwd.c_str());
    execvp(_argv[0], _argv.data());

    _exit(0);
#endif
}
