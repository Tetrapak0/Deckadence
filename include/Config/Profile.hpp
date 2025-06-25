#pragma once

#include <string>
#include <vector>

#include "../../external/jsonhpp/json.hpp"

#include "Item.hpp"

using std::vector;
using std::string;

using json = nlohmann::json;

class Client;

class Profile {
    friend class Client;
public:
    string name;
    int rows = 4;
    int columns = 6;

    json* config = nullptr;
    Client& parent;

    FolderItem items;
    FolderItem* root = &items;


    explicit Profile(Client& parent, json* config = nullptr);
private:
    string m_name = name;
    int m_rows = rows;
    int m_columns = columns;
};
