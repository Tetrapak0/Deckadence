#pragma once

#include "Item.hpp"

class FolderItem : public Item {
public:
    vector<shared_ptr<Item>> items;

    json& request_config(int idx);
    const char* get_typename() override {return "Folder";}
    void draw_properties() override;
    void properties_cancel() override;
    void properties_apply() override;
    bool disabled() override;
    bool openable_in_editor() override;
    void execute() override;

    explicit FolderItem(json* config, Profile& parent_profile, FolderItem* parent);
};

class GoUpItem : public Item {
public:
    const char* get_typename() override {return "Go up";}
    void draw_properties() override;
    void properties_cancel() override;
    void properties_apply() override;
    bool disabled() override;
    bool openable_in_editor() override;
    void execute() override;

    explicit GoUpItem(json* config, Profile& parent_profile, FolderItem* parent);
};
