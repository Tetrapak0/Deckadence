#pragma once

#define DX_ITEM_MAX 256

#include <string>

#include "../../external/jsonhpp/json.hpp"

using std::string;
using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::vector;

using json = nlohmann::json;

class Profile;

class FolderItem; // this is stupid. I hate it

// TODO: Give buttons a UUID so thumbnail assignment is easier
class Item {
public:
    friend void gui_draw_item_properties();

    string label;
    json* config = nullptr;
    Profile& parent_profile;
    FolderItem* parent = nullptr;

    virtual const char* get_typename() {return nullptr;}

    virtual void draw_properties() = 0;
    virtual void properties_cancel() = 0;
    virtual void properties_apply() = 0;
    virtual bool prop_apply_disable() = 0;
    virtual void execute() = 0;

    virtual ~Item() = default;
protected:
    // for editor
    string m_label = label;

    Item() = delete;
    explicit Item(json* config, Profile& parent_profile, FolderItem* parent) : config(config), parent_profile(parent_profile), parent(parent) {}
};

class ExecutableItem : public Item {
public:
    string command;
    string args;
    string cwd;
    bool admin = false;
    bool console = false;
#ifndef _WIN32
    vector<string> strargv;
    vector<char*> argv;
#endif
    void args_to_argv();

    explicit ExecutableItem(json* config, Profile& parent_profile, FolderItem* parent);

    const char* get_typename() override {return "Executable";}
    void draw_properties() override;
    void properties_cancel() override;
    void properties_apply() override;
    bool prop_apply_disable() override;
    void execute() override;
protected:
    string m_command = command;
    string m_args = args;
    string m_cwd = cwd;
    bool m_admin = admin;
    bool m_console = console;
};

class FilesystemAndURLItem : public Item {
public:
    string path;

    explicit FilesystemAndURLItem(json* config, Profile& parent_profile, FolderItem* parent);

    const char* get_typename() override {return "Filesystem / URL";}
    void draw_properties() override;
    void properties_cancel() override;
    void properties_apply() override;
    bool prop_apply_disable() override;
    void execute() override;
protected:
    string m_path = path;
};

class FolderItem : public Item {
public:
    vector<shared_ptr<Item>> items;

    json& request_config(int idx);
    const char* get_typename() override {return "Folder";}
    void draw_properties() override;
    void properties_cancel() override;
    void properties_apply() override;
    bool prop_apply_disable() override;
    void execute() override;

    explicit FolderItem(json* config, Profile& parent_profile, FolderItem* parent);
};

class GoUpItem : public Item {
public:
    const char* get_typename() override {return "Go up";}
    void draw_properties() override;
    void properties_cancel() override;
    void properties_apply() override;
    bool prop_apply_disable() override;
    void execute() override;

    explicit GoUpItem(json* config, Profile& parent_profile, FolderItem* parent);
};

// TODO: Add clock item/widget
// TODO: widgets
// TODO: Implement script buttons
// TODO: Add optional Windows 8 style grids
// TODO: Add keypress buttons
