#include "../../include/Client/Client.hpp"
#include "../../include/Config/Config.hpp"
#include "../../include/GUI/GUI.hpp"

[[nodiscard]] uint64_t Client::get_uuid() const {
    return m_uuid;
}

void Client::set_nickname(const string& nickname) {
    lock.lock();
    this->nickname = nickname;
    lock.unlock();
}
[[nodiscard]] string Client::get_nickname() const {
    return this->nickname;
}

[[nodiscard]] Profile& Client::get_current_profile_ref() {
    return profiles[current_profile];
}

json& Client::get_config() {
    return this->config;
}


int Client::configure() {
    this->lock.lock();
    profiles.clear();
    fs::path dkd_dir = get_cfg_dir();
    //auto lwt = std::filesystem::last_write_time(dkd_dir / (std::to_string(m_uuid)+".json"));
    //auto ch_tp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(lwt - decltype(lwt)::clock::now()
    //          + std::chrono::system_clock::now());
    //time_t lwt_time_t = std::chrono::system_clock::to_time_t(ch_tp);
    //printf("%llu\n", lwt_time_t);
    fs::create_directories(dkd_dir);
    fs::path cfg_file(dkd_dir / (std::to_string(m_uuid)+".json"));
    if (!fs::exists(cfg_file)) {
        json schema;
        schema[std::to_string(m_uuid)]["profiles"] = {};
        json& profileschema = schema[std::to_string(m_uuid)]["profiles"];
        profileschema[0]["idx"] = 0;
        profileschema[0]["name"] = "Profile";
        profileschema[0]["pages"] = {};
        profileschema[0]["rows"] = 4;
        profileschema[0]["columns"] = 6;
        json& pageschema = profileschema[0]["pages"];
        pageschema[0]["idx"] = 0;
        pageschema[0]["buttons"] = {};
        std::ofstream writer(cfg_file);
        writer << schema.dump(4);
        writer.close();
    }
    std::ifstream reader(dkd_dir / cfg_file);
    this->config = json::parse(reader);
    // TODO: JSON error checking
    reader.close();
    // TODO: Move to respective constructors, just pass json
    // TODO: Construct JSON from classes, don't store it.
    if (config.contains("nickname")) {
        this->nickname = config["nickname"];
    }
    for (auto& profile : this->config["profiles"])
        profiles.emplace_back();
    for (auto& profile : this->config["profiles"]) {
        int profile_index = profile["idx"].get<int>();
        // TODO: Delete if false
        if (profile_index < profiles.size())
            profiles[profile_index] = Profile(profile["name"].get<string>(), profile["rows"].get<int>(), profile["columns"].get<int>());
        Profile& p = profiles[profile_index];
        for (auto& page : profile["pages"]) {
            for (auto& button : page["buttons"]) {
                int idx = button["idx"].get<int>();
                if (idx > p.rows*p.columns || idx < 0)
                    continue;
                Item::type_t type = static_cast<Item::type_t>(button["type"].get<int>());
                string command = button["command"].get<string>();
                string args = button.contains("args") ? button["args"] : "";
                bool admin = button["admin"].get<bool>();
                p.items[idx] = Item(button["label"], type, command, args, admin);
            }
        }
    }
    this->lock.unlock();
    return 0;
}

// TODO: Make this for profiles as well
void Client::draw_properties() {
    Deckastore& dxstore = Deckastore::get();
    Profile& dxprofile = this->get_current_profile_ref();
    ImGui::OpenPopup("Client properties");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480, 360));
    if (ImGui::BeginPopupModal("Client properties", nullptr, modalflags)) {
        ImGui::Text("Nickname");
        ImGui::SameLine();
        ImGui::InputText("##nickname", &this->m_nickname);
        ImGui::Text("Rows");
        ImGui::SameLine();
        ImGui::InputInt("##rows", &dxprofile.m_rows);
        ImGui::Text("Columns");
        ImGui::SameLine();
        ImGui::InputInt("##columns", &dxprofile.m_columns);
        if (dxprofile.m_rows * dxprofile.m_columns > 256) {
            dxprofile.m_rows = 4;
            dxprofile.m_columns = 6;
        }
        if (dxprofile.m_rows*dxprofile.m_columns < dxprofile.rows*dxprofile.columns) {
            ImGui::TextDisabled("(i) Excess elements will be preserved until client disconnect or\n\tprogram exit");
        }
        ImGui::SetCursorPos(ImVec2(340, 326));
        if (ImGui::Button("Cancel", ImVec2(64, 26))) {
            this->m_nickname = nickname;
            dxprofile.m_rows = dxprofile.rows;
            dxprofile.m_columns = dxprofile.columns;
            dxstore.draw_properties = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Confirm", ImVec2(64, 26))) {
            this->nickname = m_nickname;
            int old_size = dxprofile.rows*dxprofile.columns;
            int new_size = dxprofile.m_rows*dxprofile.m_columns;
            if (old_size < new_size) {
                for (int i = old_size; i < new_size; ++i) {
                    dxprofile.items.emplace_back();
                }
            }
            window_resized = true;
            dxprofile.rows = dxprofile.m_rows;
            dxprofile.columns = dxprofile.m_columns;
            this->config["nickname"] = m_nickname;

            int profile_counter = 0;
            bool profile_exists = false;
            for (auto& profile : config["profiles"]) {
                if (profile["idx"] == this->current_profile) {
                    profile_exists = true;
                    profile["rows"] = dxprofile.m_rows;
                    profile["columns"] = dxprofile.m_columns;
                } else ++profile_counter;
            }
            if (!profile_exists) {
                json profile = {
                    {"idx", this->current_profile},
                    {"rows", dxprofile.rows},
                    {"columns", dxprofile.columns}
                };
                config["profiles"].emplace_back(profile);
            }

            fs::path dkd_dir = get_cfg_dir();
            std::ofstream writer(dkd_dir / (std::to_string(this->get_uuid()) + ".json"));
            writer << this->config.dump(4);
            writer.close();
            send(this->socket, string(string(1, DX_CONFIG_BYTE) + this->config.dump()).c_str(), this->config.dump().length()+1, 0);

            dxstore.draw_properties = false;
        }

        ImGui::EndPopup();
    }
}
