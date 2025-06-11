#pragma once

#include <string>

using std::string;

// TODO: Give buttons a UUID so thumbnail assignment is easier
struct Item {
    string label;
    enum class type_t {EXE, DIR, FILE, URL, CMD};
    type_t type = type_t::EXE;
    string command;
    string args;
    bool admin = false;

    void draw_properties();
    void execute() const;

    Item() = default;
    explicit Item(string lbl) : label(std::move(lbl)) {}
    explicit Item(string lbl, type_t type, const string& command, const string& args, bool admin) : label(std::move(lbl)),
                                                             type(type),
                                                             command(command),
                                                             args(args),
                                                             admin(admin),
                                                             m_label(label),
                                                             m_type(type),
                                                             m_command(command),
                                                             m_args(args),
                                                             m_admin(admin) {}
private:
    // for editor
    string m_label = label;
    type_t m_type = type;
    string m_command = command;
    string m_args = args;
    bool m_admin = admin;
};
