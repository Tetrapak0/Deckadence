#pragma once

#include <any>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using std::string;
using std::vector;

#include "Thumbnail.hpp"

namespace NxSh::GUI {
struct Item {
    // TODO: add function buttons that can call certain functions like disconnect.
    const char* const item_typename = "NULL";

    int uses_thumbnail = 0; // later make enum that can show both

    string label = "";

    std::unordered_map<string, std::any> members;

    Thumbnail thumbnail;

    // All derivative classes should include these functons so that the parameter types can mach the class's
    virtual void     set_member(string name, std::any value);
    virtual std::any get_member(string name);

    // TODO: Each getter / setter has mutex
    virtual void     set_label(string label); // Non pure virtual
    virtual string   get_label();             //

    virtual void     set_dtype(int dtype);    //
    virtual int      get_dtype();             //

    virtual int      show_properties();

    virtual int      execute(); // pure virtual

    protected:
        // This struct SHOULD NOT be instantiated.
        // You must include your own public constructor for the derived classes.
        Item();
};

class FileButton : public Item {
    std::unordered_map<string, std::any> members = {{"Path", ""}, {"Parameters", ""}};
public:
    const char* const item_typename = "FileButton";
    
    void   set_path(string path);
    string get_path();
    int    execute();

    FileButton() = default;
};

class DirButton : public Item {
    string m_path;
public:
    const char* const item_typename = "DirButton";
    void   set_path(string path);
    string get_path();
    int    execute();

    DirButton() = default;
};

class URLButton : public Item {
    string m_url;
public:
    const char* const item_typename = "URLButton";
    void   set_url(string url);
    string get_url();
    int    execute();

    URLButton() = default;
};

class Folder : public Item/*, public Page*/ {
    using Item::get_member;     // These should not be available for a folder
    using Item::set_member;     //
    using Item::members;        //
public:
    const char* const item_typename = "Folder";

    Folder* const parent = nullptr;

    vector<Item> items;

    int show_properties();

    Folder() = default;
    Folder(Folder* _parent) : parent(_parent) {}

    Item& operator[](size_t idx) {
        if (idx >= 0 && idx < items.size()) return items[idx];
        else throw(std::out_of_range("Folder::operator[]: Subscript out of range"));
    }
    const Item& operator[](size_t idx) const {
        if (idx >= 0 && idx < items.size()) return items[idx];
        else throw(std::out_of_range("Folder::operator[]: Subscript out of range"));
    }
};
}