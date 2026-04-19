#include "Config/Deckastore.hpp"
#include "Config/Config.hpp"
#include "Config/Item.hpp"
#include "Config/ExecutableItem.hpp"
#include "Config/FolderItem.hpp"
#include "Utilities/FileDialog.hpp"

FolderItem::FolderItem(json* config, Profile& parent_profile, FolderItem* parent) : Item(config, parent_profile, parent) {
    if (!this->parent_profile.root)
        this->parent_profile.root = this;
    this->items.reserve(DX_ITEMS_MAX);
    Deckastore& dxstore = Deckastore::get();
    ItemTypeRegistry& type_registry = dxstore.type_registry;
    if (config) {
        (*config)["items"].get_ref<json::array_t&>().reserve(DX_ITEMS_MAX);
        for (int i = 0; i < this->parent_profile.rows*this->parent_profile.columns; ++i) {
            this->items.emplace_back(std::make_shared<ExecutableItem>(ExecutableItem(nullptr, this->parent_profile, this)));
        }
        for (auto& item : (*config)["items"]) {
            if (!item.contains("idx"))
                continue;
            int idx = item["idx"].get<int>();
            if (idx > this->parent_profile.rows*this->parent_profile.columns || idx < 0)
                continue;
            if (!item.contains("type"))
                continue;
            string type = item["type"].get<string>();
            if (type_registry.type_exists(type.c_str())) {
                this->items[idx] = std::shared_ptr<Item>(type_registry.retrieve_type(item["type"].get<string>().c_str(), &item, this->parent_profile, this));
            } else {
                // TODO: show error, missing plugin, etc.
            }
        }
    }
}

json& FolderItem::request_config(int idx) {
    (*config)["items"].push_back({
        {"idx", idx}
    });
    return (*config)["items"].back();
}

bool FolderItem::has_behavior_settings() {
    return false;
}

void FolderItem::draw_properties() {}

void FolderItem::properties_cancel() {}

void FolderItem::properties_apply() {
    //this->parent = this->parent_profile.root; // TODO: Why? Pretty sure this is always set
    if (this->items.size() < this->parent_profile.rows*this->parent_profile.columns) {
        for (int i = this->items.size(); i < this->parent_profile.rows*this->parent_profile.columns; ++i) {
            this->items.emplace_back(std::make_shared<ExecutableItem>(ExecutableItem(nullptr, this->parent_profile, this->parent)));
        }
    }
    if (!this->config->contains("items"))
        (*this->config)["items"] = json::array();
}

bool FolderItem::disabled() {
    return false;
}

bool FolderItem::openable_in_editor() {
    return true;
}

void FolderItem::execute() {
    if (this->items.size() < this->parent_profile.rows*this->parent_profile.columns) {
        for (int i = this->items.size(); i < this->parent_profile.rows*this->parent_profile.columns; ++i) {
            this->items.emplace_back(std::make_shared<ExecutableItem>(ExecutableItem(nullptr, this->parent_profile, this)));
        }
    }
    this->parent_profile.root = this;
}



GoUpItem::GoUpItem(json* config, Profile& parent_profile, FolderItem* parent) : Item(config, parent_profile, parent) {}

void GoUpItem::draw_properties() {
    ImGui::BeginDisabled();
    ImGui::TextWrapped("(i) Right click button to go up");
    ImGui::EndDisabled();
}

bool GoUpItem::has_behavior_settings() {
    return false;
}

void GoUpItem::properties_cancel() {}

void GoUpItem::properties_apply() {}

bool GoUpItem::disabled() {
    return this->parent_profile.root->parent ? false : true;
}

bool GoUpItem::openable_in_editor() {
    return true;
}

void GoUpItem::execute() {
    if (this->parent_profile.root->parent) {
        //if (this->parent_profile.root->parent->items.size() < this->parent_profile.rows*this->parent_profile.columns) {
        //    for (int i = this->parent_profile.root->parent->items.size(); i < this->parent_profile.rows*this->parent_profile.columns; ++i) {
        //        this->parent_profile.root->parent->items.emplace_back(std::make_shared<ExecutableItem>(ExecutableItem(nullptr, this->parent_profile, this->parent)));
        //    }
        //}
        this->parent_profile.root = this->parent_profile.root->parent;
    }
}
