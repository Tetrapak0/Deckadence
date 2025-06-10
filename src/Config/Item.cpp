#include "../../include/Config/Config.hpp"
#include "../../include/Config/Deckastore.hpp"
#include "../../include/Config/Item.hpp"
#include "../../include/GUI/GUI.hpp"
#include "../../include/Utilities/FileDialog.hpp"

void Item::draw_properties() {
    Deckastore& dxstore = Deckastore::get();
    static const char* types[5] = {"Executable", "Directory", "File", "URL", "Command"};
    ImGui::OpenPopup("Button properties");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480, 360));
    if (ImGui::BeginPopupModal("Button properties", nullptr, modalflags)) {
        ImGui::Text("Label ");
        ImGui::SameLine();
        ImGui::InputText("##lbl", &this->m_label);

        ImGui::Text("Type: ");
        ImGui::SameLine();
        if (ImGui::BeginCombo("##Type", types[static_cast<int>(this->m_type)])) {
            for (int i = 0; i < IM_ARRAYSIZE(types); ++i) {
                const bool selected = static_cast<int>(this->m_type) == i;
                if (ImGui::Selectable(types[i], selected)) {
                    this->m_type = static_cast<type_t>(i);
                }
                if (selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        // TODO: Customize per type
        ImGui::Text("Command");
        ImGui::SameLine();
        ImGui::InputText("##cmd", &this->m_command);
        if (this->m_type < type_t::URL) {
            ImGui::SameLine();
            // FIXME: Fix crash when resizing if file dialog is open
            if (ImGui::Button("Browse")) {
                if (this->m_type == type_t::EXE) {
                    string res = nfd_open_exe();
                    if (!res.empty())
                        m_command = res;
                } else if (this->m_type == type_t::DIR) {
                    string res = nfd_open_dir();
                    if (!res.empty())
                        m_command = res;
                } else if (this->m_type == type_t::FILE) {
                    string res = nfd_open_file();
                    if (!res.empty())
                        m_command = res;
                }
            }
            if (this->m_type == type_t::CMD || this->m_type == type_t::EXE) {
                ImGui::Checkbox("Run with administrator rights", &this->m_admin);
            } else this->m_admin = false;
        }

        ImGui::SetCursorPos(ImVec2(340, 326));
        if (ImGui::Button("Cancel", ImVec2(64, 26))) {
            this->m_label = this->label;
            this->m_type = this->type;
            this->m_command = this->command;
            this->m_admin = this->admin;
            dxstore.draw_item_properties = -1;
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(this->m_command.empty() && !this->m_label.empty());
        if (ImGui::Button("Confirm", ImVec2(64, 26))) {
            this->label = this->m_label;
            this->type = this->m_type;
            this->command =this->m_command;
            this->admin = this->m_admin;
            Client& parent = dxstore.retrieve_current_client();
            json& config = parent.get_config();
            int profile_counter = 0;
            // int page_counter = 0;
            int button_counter  = 0;
            bool profile_exists = false;
            // bool page_exists = false;
            bool button_exists  = false;
            for (auto& profile : config["profiles"]) {
                if (profile["idx"] == parent.current_profile) {
                    profile_exists = true; // TODO: Use
                    // TODO: Change to actual page
                    // TODO: Only send required data
                    // TODO: Delete empty items
                    for (auto& button : profile["pages"][0]["buttons"]) {
                        if (button["idx"] == dxstore.draw_item_properties) {
                            button_exists = true;
                            button["label"] = this->label;
                            button["type"] = static_cast<int>(this->type);
                            button["command"] = this->command;
                            button["admin"] = this->admin;
                        } else ++button_counter;
                    }
                    if (!button_exists) {
                        json& jbuttons = profile["pages"][0]["buttons"];
                        json button = {
                            {"idx", dxstore.draw_item_properties},
                            {"label", this->label},
                            {"type", static_cast<int>(this->type)},
                            {"command", this->command},
                            {"admin", this->admin}
                        };
                        jbuttons.emplace_back(button);
                    }
                } else ++profile_counter;
            }
            fs::path dkd_dir = get_cfg_dir();
            std::ofstream writer(dkd_dir / (std::to_string(parent.get_uuid()) + ".json"));
            writer << config.dump(4);
            writer.close();
            send(parent.socket, string(string(1, DX_CONFIG_BYTE) + config.dump()).c_str(), config.dump().length()+1, 0);
            dxstore.draw_item_properties = -1;
        }
        ImGui::EndDisabled();
        ImGui::EndPopup();
    }
}

void Item::execute() const {
#ifdef _WIN32
    printf("command: %s\n", this->command.c_str());
    if ((this->type == type_t::CMD || this->type == type_t::EXE) && this->admin)
        ShellExecuteA(nullptr, "runas", this->command.c_str(), nullptr, nullptr, SW_SHOW);
    else
        ShellExecuteA(nullptr, "open", this->command.c_str(), nullptr, nullptr, SW_SHOW);
#else
    // TODO: return value
    pid_t pid = fork();
    if (pid)
        return;

    setsid();

    int fd = open("/dev/null", O_RDWR);
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    if (fd > 2)
        close(fd);

    execl(this->command.c_str(), this->command.c_str(), (char *)NULL);

    _exit(0);
#endif
}
