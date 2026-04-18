#include "../../include/Config/Profile.hpp"

#include "../../include/Utilities/SpaceStripper.hpp"

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

// TODO: Make this for properties
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
    if (uuid) {
        if (std::find(this->m_used_uuids.begin(), this->m_used_uuids.end(), uuid) == this->m_used_uuids.end()) {
            this->m_used_uuids.push_back(uuid);
            return 0;
        }
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
