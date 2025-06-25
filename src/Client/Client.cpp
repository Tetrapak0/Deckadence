#include "../../include/Client/Client.hpp"
#include "../../include/Config/Config.hpp"
#include "../../include/GUI/GUI.hpp"

Client::Client(const uint64_t uuid, const socket_t socket) : m_uuid(uuid), socket(socket) {
    if (configure()) {
        if (this->socket != NX_INVALID_SOCKET) {
            nx_sock_close(this->socket);
            this->socket = NX_INVALID_SOCKET;
        }
    }/* else {
        if (this->socket != NX_INVALID_SOCKET) {
            // TODO: Truncate redundant info from message
            string msg = string(1, DX_CONFIG_BYTE) + get_config().dump();
            send(socket, msg.c_str(), msg.length()+1, 0);
        }
    }*/
    // TODO: Check via hash
    // TODO: if client has config and server doesn't, show a dialog and send config to server
}

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
    return *profiles[current_profile];
}

json& Client::get_config() {
    return this->config;
}

int Client::configure() {
    this->lock.lock(); // TODO: Change to main mutex or implement this one better
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
        schema["profiles"] = json::array();
        json& profileschema = schema["profiles"];
        profileschema[0]["idx"] = 0;
        profileschema[0]["name"] = "Profile";
        profileschema[0]["pages"] = json::array();
        profileschema[0]["rows"] = 4;
        profileschema[0]["columns"] = 6;
        json& pageschema = profileschema[0]["pages"];
        pageschema[0]["idx"] = 0;
        pageschema[0]["items"] = json::array();
        std::ofstream writer(cfg_file);
        writer << schema.dump(4);
        writer.close();
    }
    std::ifstream reader(dkd_dir / cfg_file);
    this->config = json::parse(reader);
    // TODO: JSON error checking
    reader.close();
    if (config.contains("nickname")) {
        this->nickname = config["nickname"];
        this->m_nickname = nickname;
    }
    profiles.resize(this->config["profiles"].size());
    for (auto& profile : this->config["profiles"]) {
        int profile_index = profile["idx"].get<int>();
        if (profile_index < profiles.capacity())
            profiles[profile_index] = std::make_shared<Profile>(*this, &profile);
    }
    this->lock.unlock();
    return 0;
}

void Client::update_config() {
    fs::path dkd_dir = get_cfg_dir();
    std::ofstream writer(dkd_dir / (std::to_string(m_uuid) + ".json"));
    writer << config.dump(4);
    writer.close();
    string msg = string(1, DX_CONFIG_BYTE) + config.dump();
    send(socket, msg.c_str(), msg.length()+1, 0);
    // string whole_nav;
    // for (auto& nav : nav_history) {
        // send(socket, nav.c_str(), nav.length()+1, 0);
        // whole_nav += nav + '\0';
    // }
    // send(socket, whole_nav.c_str(), whole_nav.size()+1, 0);
}

// TODO: Make this for profiles as well
void Client::draw_properties() {
    Deckastore& dxstore = Deckastore::get();
    Profile& dxprofile = this->get_current_profile_ref();
    ImGui::OpenPopup("Client properties");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480, 360));
    if (ImGui::BeginPopupModal("Client properties", nullptr, modalflags)) {
        ImGui::TextDisabled("Client ID: %llu", this->m_uuid);
        ImGui::Text("Nickname");
        ImGui::SameLine();
        ImGui::InputText("##nickname", &this->m_nickname);
        ImGui::Text("Rows");
        ImGui::SameLine();
        ImGui::InputInt("##rows", &dxprofile.m_rows);
        if (dxprofile.m_rows * dxprofile.m_columns > 256) {
            dxprofile.m_rows = dxprofile.rows;
        }
        ImGui::Text("Columns");
        ImGui::SameLine();
        ImGui::InputInt("##columns", &dxprofile.m_columns);
        if (dxprofile.m_rows * dxprofile.m_columns > 256) {
            dxprofile.m_columns = dxprofile.columns;
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
            const int old_size = dxprofile.rows*dxprofile.columns;
            const int new_size = dxprofile.m_rows*dxprofile.m_columns;
            if (old_size < new_size) {
                for (int i = old_size; i < new_size; ++i) {
                    dxprofile.root->items.emplace_back(std::make_shared<ExecutableItem>(ExecutableItem(nullptr, dxprofile, dxprofile.root)));
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
            // TODO: Reconfigure self if profile is emplaced as to not invalidate references
            if (!profile_exists) {
                json profile = {
                    {"idx", this->current_profile},
                    {"rows", dxprofile.rows},
                    {"columns", dxprofile.columns}
                };
                config["profiles"].emplace_back(profile);
            }
            // FIXME: nav_history
            // Profile& p = this->get_current_profile_ref();
            // if (p.items.items.size() < p.rows*p.columns) {
            //     for (int i = p.items.items.size(); i < p.rows*p.columns; ++i) {
            //         p.items.items.emplace_back(std::make_shared<ExecutableItem>(ExecutableItem(nullptr, p, &p.items)));
            //     }
            // }
            // this->get_current_profile_ref().root = &this->get_current_profile_ref().items;
            this->update_config();

            dxstore.draw_properties = false;
        }

        ImGui::EndPopup();
    }
}
