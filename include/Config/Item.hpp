#pragma once

#define DX_ITEMS_MAX 256

#include <string>

#include "../../external/jsonhpp/json.hpp"

#include "../GUI/Texture.hpp"

using std::string;
using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::vector;

using json = nlohmann::json;

class Profile;
class FolderItem;

class Item {
public:
    string label;
    json* config = nullptr;
    Profile& parent_profile;
    FolderItem* parent = nullptr;

    bool has_thumbnail = false;
    shared_ptr<Texture> thumbnail = std::make_shared<Texture>(Texture());

    // bool custom_style = false;
    // TODO: widgets
    // TODO: Add clock item/widget
    // Vec2<int> u_size; // Size in button units // TODO: Windows 8 style grids

    virtual const char* get_typename() {return nullptr;}

    virtual void draw_properties();
    virtual void properties_cancel();
    void         properties_init(const char* _typename);
    virtual void properties_apply();
    // This function specifies whether the "Apply" button in the properties window
    //      is enabled or disabled. It also specifies whether the button itself is
    //      enabled or disabled.
    virtual bool disabled() = 0;
    virtual bool has_behavior_settings() {return true;};
    Item*        copy_properties(Item& other);
    virtual bool openable_in_editor();
    uint64_t     get_uuid() const;
    int          set_uuid(uint64_t uuid);
    // virtual void draw();
    virtual void execute() = 0;

    virtual ~Item();

    Item(const Item& other);
    explicit Item(const Item* other);
    Item(const Item&& other) noexcept;

    Item& operator=(const Item& other);
protected:
    // for editor // TODO: Maybe make private
    string m_label = label;
    bool m_has_thumbnail = has_thumbnail;
    shared_ptr<Texture> m_thumbnail = thumbnail;

    Item() = delete;
    explicit Item(json* config, Profile& parent_profile, FolderItem* parent);
private:
    uint64_t m_uuid = 0;
    bool m_uuid_set = false;
};

// TODO: Implement multi-action buttons
// TODO: Add keypress buttons
// TODO: Make FolderItem and any other such widgets Views
