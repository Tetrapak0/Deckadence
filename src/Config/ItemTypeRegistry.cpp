#include "Client/Client.hpp"
#include "Config/ItemTypeRegistry.hpp"
#include "Config/ExecutableItem.hpp"
#include "Config/FilesystemAndURLItem.hpp"
#include "Config/FolderItem.hpp"
#include "Config/Profile.hpp"

ItemTypeRegistry& ItemTypeRegistry::get() {
    static ItemTypeRegistry itr;
    return itr;
}

const unordered_map<string, ItemTypeRegistry::constructor_t>& ItemTypeRegistry::get_registry() {
    return itemtypestore;
}

void ItemTypeRegistry::register_type(const char* const item_typename, const constructor_t& constructor) {
    if (!type_exists(item_typename))
        itemtypestore.emplace(item_typename, constructor);
}

Item* ItemTypeRegistry::retrieve_type(const char* const item_typename, json* config, Profile& parent_profile, FolderItem* parent) {
    if (itemtypestore.find(item_typename) == itemtypestore.end())
        return nullptr;
    return itemtypestore[item_typename](config, parent_profile, parent);
}

bool ItemTypeRegistry::type_exists(const char* const item_typename) const {
    return this->itemtypestore.find(item_typename) != this->itemtypestore.end();
}

void ItemTypeRegistry::reset() {
    this->itemtypestore.clear();
    // TODO: this is so flawed
    // TODO: Create dummy client with id 0 to hold all of these
    // TODO: Should probably replace unordered_map
    register_type("Go up", [](json* j, Profile& p, FolderItem* fi){return new GoUpItem(j, p, fi);});
    register_type("Folder", [](json* j, Profile& p, FolderItem* fi){return new FolderItem(j, p, fi);});
    register_type("Filesystem / URL", [](json* j, Profile& p, FolderItem* fi){return new FilesystemAndURLItem(j, p, fi);});
    register_type("Executable", [](json* j, Profile& p, FolderItem* fi){return new ExecutableItem(j, p, fi);});
}
