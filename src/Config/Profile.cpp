#include "../../include/Config/Deckastore.hpp"
#include "../../include/Config/Profile.hpp"

// it just keeps getting messier and messier...
Profile::Profile(Client& parent, json* config) : rows(config ?
                                                      config->contains("rows") && (*config)["rows"].is_number_integer() ?
                                                          config->contains("columns") && (*config)["columns"].is_number_integer() ?
                                                              (*config)["rows"].get<uint64_t>() * (*config)["columns"].get<uint64_t>() > 256 ?
                                                                  16 :
                                                                (*config)["rows"].get<int>() :
                                                          (*config)["rows"].get<int>() :
                                                      4 :
                                                      4),
                                                columns(config ?
                                                      config->contains("columns") && (*config)["columns"].is_number_integer() ?
                                                          config->contains("rows") && (*config)["rows"].is_number_integer() ?
                                                              (*config)["rows"].get<uint64_t>() * (*config)["columns"].get<uint64_t>() > 256 ?
                                                                  16 :
                                                                (*config)["columns"].get<int>() :
                                                          (*config)["columns"].get<int>() :
                                                      6 :
                                                      6),
                                                 config(config), parent(parent),
                                                 // TODO: This can end badly, fix
                                                 items(config ?
                                                       &(*config)["pages"][0] :
                                                       nullptr, *this, nullptr) {
    if (config->contains("name")) {
        if ((*config)["name"].is_string()) {
            this->name = (*config)["name"].get<string>();
            this->m_name = name;
        }
    }
    this->m_columns = columns;
    this->m_rows = rows;
    // for (auto& page : (*config)["pages"]) {
    // }
}
