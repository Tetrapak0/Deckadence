#include "Client/Client.hpp"
#include "Config/Config.hpp"
#include "Config/ExecutableItem.hpp"
#include "GUI/GUI.hpp"
#include "Server/Message.hpp"

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

void Client::create_config() {
    config["profiles"] = json::array();
    json& profileschema = config["profiles"];
    profileschema[0]["idx"] = 0;
    profileschema[0]["name"] = "Profile";
    profileschema[0]["pages"] = json::array();
    profileschema[0]["rows"] = 4;
    profileschema[0]["columns"] = 6;
    json& pageschema = profileschema[0]["pages"];
    pageschema[0]["idx"] = 0;
    pageschema[0]["items"] = json::array();
    fs::path cfg_file(get_cfg_dir() / (std::to_string(m_uuid)+".json"));
    std::ofstream writer(cfg_file);
    writer << config.dump(4);
    writer.close();
    profiles.reserve(1);
}

int Client::configure() {
    this->lock.lock(); // TODO: Change to main mutex or implement this one better
    profiles.clear();

    fs::path dkd_dir = get_cfg_dir();
    fs::create_directories(dkd_dir);
    fs::path cfg_file(dkd_dir / (std::to_string(m_uuid)+".json"));
    if (!fs::exists(cfg_file))
        create_config();

    std::ifstream reader(dkd_dir / cfg_file);
    this->config = json::parse(reader, nullptr, false);
    if (this->config.is_discarded()) {
        reader.close();
        this->lock.unlock();
        return -1;
    }
    reader.close();

    if (config.contains("nickname")) {
        this->nickname = config["nickname"];
        this->m_nickname = nickname;
    }
    if (this->config.contains("profiles") &&
        this->config["profiles"].is_array() && !this->config["profiles"].empty()) {
        profiles.resize(this->config["profiles"].size());
    } else {
        create_config();
    }
    for (auto& profile : this->config["profiles"]) {
        int profile_index = -1;
        if (profile.contains("idx")) {
            profile_index = profile["idx"].get<int>();
        } else {
            profile_index = profiles.size();
            profile["idx"] = profile_index;
        }
        if (!profile.contains("pages") || !profile["pages"].is_array() || profile["pages"].empty()) {
            profile["pages"] = json::array();
            profile["pages"][0]["idx"] = 0;
            profile["pages"][0]["items"] = json::array();
        }
        // TODO: Move this to profile later
        for (int i = 0; i < profile["pages"].size(); ++i) {
            json& page = profile["pages"][i];
            if (!page.contains("idx") || !page["idx"].is_number_unsigned())
                page["idx"] = i;
            if (!page.contains("items") || !page["items"].is_array())
                page["items"] = json::array();
        }
        profiles[profile_index] = std::make_shared<Profile>(*this, &profile);
    }
    this->lock.unlock();
    return 0;
}

uint64_t construct_header(uint32_t msg_len, uint32_t type) {
    return (uint64_t)htonl(msg_len) << 32 | htonl(type);
}

void Client::write_config() const {
    fs::path dkd_dir = get_cfg_dir();
    fs::create_directories(dkd_dir);
    std::ofstream writer(dkd_dir / (std::to_string(m_uuid) + ".json"));
    writer << config.dump(4);
    writer.close();
}

void Client::update_config() const {
    write_config();
    this->profiles[this->current_profile]->root = &this->profiles[this->current_profile]->items;

    json out = this->config;
    out["config_type"] = "full";

    string cfg_string = out.dump();
    uint64_t header = construct_header(cfg_string.length(), static_cast<uint32_t>(MessageType::Config));
    string msg;
    msg.reserve(sizeof(uint64_t) + cfg_string.length());
    msg.append(reinterpret_cast<const char*>(&header), sizeof(uint64_t));
    msg.append(cfg_string);
    send(socket, msg.c_str(), msg.length(), 0);
}

void Client::update_config(Item* item) const {
    write_config();

    json out = *item->config;
    out["config_type"] = "partial";

    string cfg_string = out.dump();
    uint64_t header = construct_header(cfg_string.length(), static_cast<uint32_t>(MessageType::Config));
    string msg;
    msg.reserve(sizeof(uint64_t) + cfg_string.length());
    msg.append(reinterpret_cast<const char*>(&header), sizeof(uint64_t));
    msg.append(cfg_string);
    printf("%s\n", cfg_string.c_str());
    send(socket, msg.c_str(), msg.length(), 0);
}

// TODO: Make this for profiles as well
void Client::draw_properties() {
    Deckastore& dxstore = Deckastore::get();
    Profile& dxprofile = this->get_current_profile_ref();
    if (!ImGui::IsPopupOpen("Client properties"))
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
        static constexpr unsigned char step      = 1;
        static constexpr unsigned char step_fast = 10;
        ImGui::InputScalar("##rows", ImGuiDataType_U8, &dxprofile.m_rows, &step, &step_fast, "%d");
        if (dxprofile.m_rows * dxprofile.m_columns > 256)
            dxprofile.m_rows = dxprofile.rows;
        ImGui::Text("Columns");
        ImGui::SameLine();
        ImGui::InputScalar("##columns", ImGuiDataType_U8, &dxprofile.m_columns, &step, &step_fast, "%d");
        if (dxprofile.m_rows * dxprofile.m_columns > 256)
            dxprofile.m_columns = dxprofile.columns;
        if (dxprofile.m_rows*dxprofile.m_columns < dxprofile.rows*dxprofile.columns)
            ImGui::TextDisabled("(i) Excess elements will be preserved until client disconnect or\n\tprogram exit");
        ImGui::SetCursorPos(ImVec2(340, 326));
        if (ImGui::Button("Cancel", ImVec2(64, 26))) {
            this->m_nickname = nickname;
            dxprofile.m_rows = dxprofile.rows;
            dxprofile.m_columns = dxprofile.columns;
            ImGui::CloseCurrentPopup();
            dxstore.draw_properties = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Confirm", ImVec2(64, 26))) {
            dxstore.lock.lock();
            this->nickname = m_nickname;
            const int old_size = dxprofile.rows*dxprofile.columns;
            const int new_size = dxprofile.m_rows*dxprofile.m_columns;
            if (old_size < new_size) {
                for (int i = old_size; i < new_size; ++i) {
                    dxprofile.root->items.emplace_back(std::make_shared<ExecutableItem>(ExecutableItem(nullptr, dxprofile, dxprofile.root)));
                }
            }
            dxprofile.rows = dxprofile.m_rows;
            dxprofile.columns = dxprofile.m_columns;
            this->config["nickname"] = m_nickname;

            json& cfg = *dxprofile.config;
            cfg["rows"] = dxprofile.rows;
            cfg["columns"] = dxprofile.columns;

            // int profile_counter = 0;
            // bool profile_exists = false;
            // for (auto& profile : config["profiles"]) {
            //     if (profile["idx"] == this->current_profile) {
            //         profile_exists = true;
            //         profile["rows"] = dxprofile.m_rows;
            //         profile["columns"] = dxprofile.m_columns;
            //     } else ++profile_counter;
            // }
            // // TODO: Reconfigure self if profile is emplaced to not invalidate references
            // if (!profile_exists) {
            //     json profile = {
            //         {"idx", this->current_profile},
            //         {"rows", dxprofile.rows},
            //         {"columns", dxprofile.columns}
            //     };
            //     config["profiles"].emplace_back(profile);
            // }
            this->update_config();

            ImGui::CloseCurrentPopup();
            dxstore.draw_properties = false;
            dxstore.lock.unlock();
        }

        ImGui::EndPopup();
    }
}
