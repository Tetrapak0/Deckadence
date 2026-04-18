#pragma once

#include "Item.hpp"

class FilesystemAndURLItem : public Item {
public:
    string path;

    explicit FilesystemAndURLItem(json* config, Profile& parent_profile, FolderItem* parent);

    const char* get_typename() override {return "Filesystem / URL";}
    void draw_properties() override;
    void properties_cancel() override;
    void properties_apply() override;
    bool disabled() override;
    void execute() override;
protected:
    string m_path = path;
};

