#pragma once

#include <string>
#include <vector>

#include "../../external/jsonhpp/json.hpp"

#include "FolderItem.hpp"

using std::string;
using std::unordered_map;
using std::vector;

using json = nlohmann::json;

class Client;

class Profile {
    friend class Client;
public:
    json* config = nullptr;
    Client& parent;

    string name;
    uint8_t rows = 4;
    uint8_t columns = 6;
private:
    string m_name = name;
    int m_rows = rows;
    int m_columns = columns;

    vector<uint64_t> m_used_uuids;
    unordered_map<uint64_t, Texture*> pendingTextures;

    string  compute_name()    const;
    uint8_t compute_rows()    const;
    uint8_t compute_columns() const;
public:
    FolderItem* root = nullptr;
    FolderItem  items;

    int  register_uuid(uint64_t uuid);
    void unregister_uuid(uint64_t uuid);
    void request_thumbnails();
    void enqueue_thumbnail(uint64_t uuid, Texture* texture);
    void handle_thumbnail_delivery(string& msg);
    inline fs::path get_profile_dir();

    explicit Profile(Client& parent, json* config = nullptr);
};
