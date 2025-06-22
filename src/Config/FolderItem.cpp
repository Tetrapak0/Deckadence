#include "../../include/Config/Config.hpp"
#include "../../include/Config/Deckastore.hpp"
#include "../../include/Config/Item.hpp"
#include "../../include/GUI/GUI.hpp"
#include "../../include/Utilities/FileDialog.hpp"

FolderItem::FolderItem(json* config, Profile& parent_profile, FolderItem* parent) : Item(config, parent_profile, parent) {
    // TODO: Force go up button by default
    this->items.reserve(DX_ITEM_MAX);
    Deckastore& dxstore = Deckastore::get();
    ItemTypeRegistry& type_registry = dxstore.type_registry;
    if (config) {
        (*config)["items"].get_ref<json::array_t&>().reserve(DX_ITEM_MAX);
        for (int i = 0; i < this->parent_profile.rows*this->parent_profile.columns; ++i) {
            this->items.emplace_back(std::make_shared<ExecutableItem>(ExecutableItem(nullptr, this->parent_profile, this)));
        }
        if (config->contains("label") && (*config)["label"].is_string()) {
            this->label = (*config)["label"].get<string>();
            this->m_label = label;
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

void FolderItem::draw_properties() {
    ImGui::AlignTextToFramePadding();
    ImGui::BeginDisabled();
    ImGui::TextWrapped("(i) Right click folder to open in editor");
    ImGui::EndDisabled();
}

void FolderItem::properties_cancel() {
    this->m_label = label;
}

void FolderItem::properties_apply() {
    this->label = m_label;
    this->parent = this->parent_profile.root;
    if (this->items.size() < this->parent_profile.rows*this->parent_profile.columns) {
        for (int i = this->items.size(); i < this->parent_profile.rows*this->parent_profile.columns; ++i) {
            this->items.emplace_back(std::make_shared<ExecutableItem>(ExecutableItem(nullptr, this->parent_profile, this->parent)));
        }
    }
    int idx = Deckastore::get().draw_item_properties;
    if (!config)
        this->config = &this->parent->request_config(idx);
    else if (!this->config->contains("items"))
        this->config->clear();
    (*this->config)["idx"] = idx;
    (*this->config)["label"] = this->label;
    (*this->config)["type"] = this->get_typename();
    if (!this->config->contains("items"))
        (*this->config)["items"] = json::array();
    this->parent_profile.parent.update_config();
}

bool FolderItem::prop_apply_disable() {
    return false;
}

void FolderItem::execute() {
    if (this->items.size() < this->parent_profile.rows*this->parent_profile.columns) {
        for (int i = this->items.size(); i < this->parent_profile.rows*this->parent_profile.columns; ++i) {
            this->items.emplace_back(std::make_shared<ExecutableItem>(ExecutableItem(nullptr, this->parent_profile, this)));
        }
    }
    this->parent_profile.root = this;
}



GoUpItem::GoUpItem(json* config, Profile& parent_profile, FolderItem* parent) : Item(config, parent_profile, parent) {
    if (config) {
        if (config->contains("label") && (*config)["label"].is_string()) {
            this->label = (*config)["label"].get<string>();
            this->m_label = label;
        }
    }
}

void GoUpItem::draw_properties() {
    ImGui::BeginDisabled();
    ImGui::TextWrapped("(i) Right click button to go up");
    ImGui::EndDisabled();
}

void GoUpItem::properties_cancel() {
    this->m_label = label;
}

void GoUpItem::properties_apply() {
    this->label = m_label;
    int idx = Deckastore::get().draw_item_properties;
    if (!config)
        this->config = &this->parent->request_config(idx);
    else
        this->config->clear();
    (*this->config)["idx"] = idx;
    (*this->config)["label"] = this->label;
    (*this->config)["type"] = this->get_typename();

    this->parent_profile.parent.update_config();
}

bool GoUpItem::prop_apply_disable() {
    return this->parent_profile.root->parent ? false : true;
}

void GoUpItem::execute() {
    if (this->parent_profile.root->parent) {
        if (this->parent_profile.root->parent->items.size() < this->parent_profile.rows*this->parent_profile.columns) {
            for (int i = this->parent_profile.root->parent->items.size(); i < this->parent_profile.rows*this->parent_profile.columns; ++i) {
                this->parent_profile.root->parent->items.emplace_back(std::make_shared<ExecutableItem>(ExecutableItem(nullptr, this->parent_profile, this->parent)));
            }
        }
        this->parent_profile.root = this->parent_profile.root->parent;
    }
}
