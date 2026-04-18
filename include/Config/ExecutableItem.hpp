#pragma once

#include "Item.hpp"

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
    bool disabled() override;
    void execute() override;
protected:
    string m_command = command;
    string m_args = args;
    string m_cwd = cwd;
    bool m_admin = admin;
    bool m_console = console;
};
