#pragma once

#include <unordered_map>
#include <functional>

#include "Item.hpp"

using std::string;
using std::unordered_map;

class ItemTypeRegistry {
    unordered_map<string, std::function<Item*(json*, Profile&, FolderItem*)>> itemtypestore;
public:
    static ItemTypeRegistry& get();
    const unordered_map<string, std::function<Item*(json*, Profile&, FolderItem*)>>& get_registry();
    void register_type(const char* item_typename, const std::function<Item*(json*, Profile&, FolderItem*)>& constructor);
    Item* retrieve_type(const char* item_typename, json* config, Profile& parent_profile, FolderItem* parent);
    bool type_exists(const char* item_typename) const;
    void reset();
protected:
    ItemTypeRegistry() {
        this->reset();
    };
};
