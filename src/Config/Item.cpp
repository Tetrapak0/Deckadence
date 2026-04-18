#include "Config/Config.hpp"
#include "Config/Item.hpp"
#include "Utilities/FileDialog.hpp"
#include "Server/Message.hpp"

Item::Item(json* config, Profile& parent_profile, FolderItem* parent) : config(config), parent_profile(parent_profile), parent(parent) {
    if (config) {
        if (config->contains("label") && (*config)["label"].is_string()) {
            this->label = (*config)["label"].get<string>();
            this->m_label = label;
        }
        if (config->contains("has_thumbnail") && (*config)["has_thumbnail"].is_boolean()) {
            this->has_thumbnail = (*config)["has_thumbnail"].get<bool>();
            this->m_has_thumbnail = this->has_thumbnail;
        }
        if (config->contains("uuid") && (*config)["uuid"].is_number_unsigned()) {
            this->m_uuid = (*config)["uuid"].get<uint64_t>();
            this->parent_profile.register_uuid(this->m_uuid);
        }
        if (this->has_thumbnail) {
            fs::path img_path = get_cfg_dir() / std::to_string(parent_profile.parent.get_uuid()) / this->parent_profile.name / string(std::to_string(this->m_uuid) + ".png");
            if (fs::exists(img_path)) {
                this->thumbnail->create_from_file(img_path.generic_string());
            } else if (Deckastore::get().get_mode() == Deckadence::mode_t::Client) {
                if ((Deckastore::get().get_tasks() & tasks::Client) != tasks::None) {
                    uint64_t header = construct_header(sizeof(uint64_t), static_cast<uint32_t>(MessageType::ThumbnailRequest));
                    printf("%llu\n", header);
                    string msg;
                    msg.reserve(2 * sizeof(uint64_t));
                    msg.append(reinterpret_cast<const char*>(&header), sizeof(uint64_t));
                    msg.append(reinterpret_cast<const char*>(&this->m_uuid), sizeof(uint64_t));
                    this->parent_profile.parent.pendingTextures[this->m_uuid] = this->thumbnail.get();
                    this->parent_profile.parent.send_res = send(this->parent_profile.parent.socket, msg.data(), msg.length(), 0);
                }
            }
        }
    }
}

void Item::draw_properties() {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Label: ");
    ImGui::SameLine();
    // TODO: Multiline label support
    ImGui::InputText("##lbl", &this->m_label);
    ImGui::Checkbox("Use image instead", &this->m_has_thumbnail);
    ImGui::SameLine();
    ImGui::BeginDisabled(!this->m_has_thumbnail);
    if (ImGui::Button("Browse", ImVec2(64, 26))) {
        string img_path = nfd_open_image();
        if (!img_path.empty()) {
            this->m_thumbnail = std::make_shared<Texture>();
            this->m_thumbnail->create_from_file(img_path);
        }
    }
    ImGui::EndDisabled();
    if (this->m_has_thumbnail) {
        if (this->m_thumbnail->ready_to_bind())
            this->m_thumbnail->bind_to_context();
        if (this->m_thumbnail->is_init()) {
            float image_aspect = (float)this->m_thumbnail->get_width() / (float)this->m_thumbnail->get_height();
            float available_aspect = 622.0f / 283.0f;
            ImVec2 size_max;
            if (available_aspect > image_aspect) {
                size_max.y = 283.0f;
                size_max.x = image_aspect * 283.0f;
            } else {
                size_max.x = 622.0f;
                size_max.y = 622.0f / image_aspect;
            }
            ImGui::Image(*this->m_thumbnail->get(), ImVec2(size_max.x, size_max.y));
        }
    }
}

void Item::properties_cancel() {
    this->m_label = this->label;
    this->m_has_thumbnail = this->has_thumbnail;
    this->m_thumbnail = this->thumbnail;
}

void Item::properties_init(const char* _typename) {
    int idx = Deckastore::get().draw_item_properties;
    if (!config)
        this->config = &this->parent->request_config(idx);
    else if (string(_typename) != this->parent_profile.root->items[idx]->get_typename())
        this->config->clear();
}

void Item::properties_apply() {
    int idx = Deckastore::get().draw_item_properties;
    this->label = this->m_label;
    this->has_thumbnail = this->m_has_thumbnail;
    if (!this->has_thumbnail) {
        this->m_thumbnail = this->thumbnail;
    } else {
        if (this->m_thumbnail != this->thumbnail) {
            fs::path profileDir(get_cfg_dir() / std::to_string(this->parent_profile.parent.get_uuid()) / this->parent_profile.name);
            fs::create_directories(profileDir);
            this->m_thumbnail->write_png((profileDir / (std::to_string(this->get_uuid()) + ".png")).generic_string().c_str());
        }
        this->thumbnail = this->m_thumbnail;
    }
    if (!config)
        this->config = &this->parent->request_config(idx);
    (*this->config)["idx"] = idx;
    (*this->config)["type"] = this->parent->items[idx]->get_typename();
    (*this->config)["has_thumbnail"] = this->has_thumbnail;
    (*this->config)["label"] = this->label;
    (*this->config)["uuid"] = this->m_uuid;
}

Item* Item::copy_properties(Item& other) {
    if (this != &other) {
        this->m_label = other.label;
        this->parent = other.parent;
        this->m_has_thumbnail = other.m_has_thumbnail;
        this->m_thumbnail = other.thumbnail;
        this->m_uuid = other.m_uuid;
    }
    return this;
}

bool Item::openable_in_editor() {
    return false;
}

uint64_t Item::get_uuid() const {
    return this->m_uuid;
}

int Item::set_uuid(uint64_t uuid) {
    assert(!this->m_uuid);
    this->m_uuid = uuid;
    if (this->parent_profile.register_uuid(uuid)) {
        this->m_uuid = 0;
        return -1;
    }
    return 0;
}

Item::Item(const Item& other) : label(other.label),
                                config(other.config),
                                parent_profile(other.parent_profile),
                                parent(other.parent),
                                has_thumbnail(other.has_thumbnail),
                                thumbnail(other.thumbnail),
                                m_label(label),
                                m_has_thumbnail(has_thumbnail),
                                m_thumbnail(thumbnail),
                                m_uuid(other.m_uuid) {}
Item::Item(const Item* other) : label(other->label),
                                config(other->config),
                                parent_profile(other->parent_profile),
                                parent(other->parent),
                                has_thumbnail(other->has_thumbnail),
                                thumbnail(other->thumbnail),
                                m_label(label),
                                m_has_thumbnail(has_thumbnail),
                                m_thumbnail(thumbnail),
                                m_uuid(other->m_uuid) {}
Item::Item(const Item&& other) noexcept :   label(other.label),
                                            config(other.config),
                                            parent_profile(other.parent_profile),
                                            parent(other.parent),
                                            has_thumbnail(other.has_thumbnail),
                                            thumbnail(other.thumbnail),
                                            m_label(label),
                                            m_has_thumbnail(has_thumbnail),
                                            m_thumbnail(thumbnail),
                                            m_uuid(other.m_uuid) {}
Item& Item::operator=(const Item& other) {
    if (this != &other) {
        this->label = other.label;
        this->m_label = other.label;
        this->config = other.config;
        this->parent = other.parent;
        this->has_thumbnail = other.has_thumbnail;
        this->m_has_thumbnail = other.m_has_thumbnail;
        this->thumbnail = other.thumbnail;
        this->m_thumbnail = other.thumbnail;
        this->m_uuid = other.m_uuid;
    }
    return *this;
}

Item::~Item() {
    if (this->m_uuid)
        this->parent_profile.unregister_uuid(m_uuid);
}
