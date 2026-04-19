#include "Config/Profile.hpp"
#include "Client/CLient.hpp"
#include "Server/Message.hpp"
#include "Utilities/SpaceStripper.hpp"

Profile::Profile(Client& parent, json* config) :    config(config),
                                                    parent(parent),
                                                    name(compute_name()),
                                                    rows(compute_rows()),
                                                    columns(compute_columns()),
                                                    m_name(name),
                                                    items(&(*config)["pages"][0], *this, nullptr) {
    // for (auto& page : (*config)["pages"]) {
    //     pages.emplace_back(&page, *this, nullptr);
    // }
    fs::create_directories(this->get_profile_dir());
    request_thumbnails();
}

void Profile::request_thumbnails() {
    uint64_t header = construct_header(sizeof(uint64_t), static_cast<uint32_t>(MessageType::ThumbnailRequest));
    for (auto& [uuid, texutere] : pendingTextures) {
        string msg;
        msg.reserve(2 * sizeof(uint64_t));
        msg.append(reinterpret_cast<const char*>(&header), sizeof(uint64_t));
        msg.append(reinterpret_cast<const char*>(&uuid), sizeof(uint64_t));
        this->parent.send_res = send(this->parent.socket, msg.data(), msg.length(), 0);
    }
}

void Profile::enqueue_thumbnail(uint64_t uuid, Texture* texture) {
    this->pendingTextures[uuid] = texture;
}

inline fs::path Profile::get_profile_dir() {
    return fs::path(get_cfg_dir()) / std::to_string(this->parent.get_uuid()) / this->name;
}

void Profile::handle_thumbnail_delivery(string& msg) {
    uint64_t uuid = 0;
    memcpy(&uuid, msg.data(), sizeof(uint64_t));
    fs::path profileDir = this->get_profile_dir();
    fs::create_directories(profileDir);
    printf("Received thumbnail for %llu\n", uuid);
    if (!pendingTextures.contains(uuid))
        return;
    msg.erase(msg.begin(), msg.begin() + sizeof(uint64_t));
    if (!memcmp(msg.data(), "NUL", 3)) {
        pendingTextures.erase(uuid);
        return;
    }
    pendingTextures[uuid]->create_from_memory(
        reinterpret_cast<const uint8_t*>(msg.data()),
        msg.size(),
        fs::path(profileDir / (std::to_string(uuid) + ".png")).generic_string().c_str());
    pendingTextures.erase(uuid);
}

string Profile::compute_name() const {
    if (config->contains("name") && (*config)["name"].is_string()) {
        string stripped = strip_spaces((*config)["name"]);
        if (stripped.empty())
            return "Profile" + std::to_string((*config)["idx"].get<int>());
        return stripped;
    }
    return "Profile" + std::to_string((*config)["idx"].get<int>());
}

uint8_t Profile::compute_rows() const {
    uint64_t t_rows = 6;
    uint64_t t_columns = 4;
    if (config->contains("rows") && (*config)["rows"].is_number_unsigned())
        t_rows = (*config)["rows"];
    if (config->contains("columns") && (*config)["columns"].is_number_unsigned())
        t_columns = (*config)["columns"];
    if (t_rows*t_columns > DX_ITEMS_MAX)
        return 16;
    return t_rows;
}

uint8_t Profile::compute_columns() const {
    uint64_t t_rows = 6;
    uint64_t t_columns = 4;
    if (config->contains("rows") && (*config)["rows"].is_number_unsigned())
        t_rows = (*config)["rows"];
    if (config->contains("columns") && (*config)["columns"].is_number_unsigned())
        t_columns = (*config)["columns"];
    if (t_rows*t_columns > DX_ITEMS_MAX)
        return 16;
    return t_columns;
}

int Profile::register_uuid(uint64_t uuid) {
    if (std::find(this->m_used_uuids.begin(), this->m_used_uuids.end(), uuid) == this->m_used_uuids.end()) {
        this->m_used_uuids.push_back(uuid);
        return 0;
    }
    return -1;
}

void Profile::unregister_uuid(uint64_t uuid) {
    if (uuid) {
        auto idx = std::find(this->m_used_uuids.begin(), this->m_used_uuids.end(), uuid);
        if (idx != this->m_used_uuids.end())
            this->m_used_uuids.erase(idx);
    }
}
