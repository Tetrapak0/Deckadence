#include "Config/Config.hpp"
#include "Config/Item.hpp"
#include "Utilities/FileDialog.hpp"
#include "Server/Message.hpp"
#include "Utilities/SHA256.hpp"

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
        if (config->contains("uuid") && (*config)["uuid"].is_number()) {
            this->parent_profile.unregister_uuid((*config)["uuid"].get<uint64_t>());
            this->set_uuid((*config)["uuid"].get<uint64_t>());
        }
        while (!m_uuid_set) {
            this->set_uuid(generate_uuid());
        }
        if (this->has_thumbnail) {
            fs::path img_path = get_cfg_dir() / std::to_string(parent_profile.parent.get_uuid()) / this->parent_profile.name / string(std::to_string(this->m_uuid) + ".png");
            if (fs::exists(img_path)) {
                this->thumbnail->create_from_file(img_path.generic_string());

                SHA256 hash;
                std::ifstream reader(img_path, std::ios::binary | std::ios::ate);

                std::streamsize len = reader.tellg();
                reader.seekg(0, std::ios::beg);

                vector<uint8_t> data(len);

                reader.read(reinterpret_cast<char*>(data.data()), len);
                reader.close();
                
                hash.update(data.data(), len);

                string hash_str = hash.toString(hash.digest());

                if (config->contains("thumbnail_hash")) {
                    if ((*config)["thumbnail_hash"] != hash_str) {
                        printf("Thumbnail hashes for %llu don't match:\n\tJSON: %s\n\tFile: %s\n", this->m_uuid, (*config)["thumbnail_hash"].get<string>().c_str(), hash_str.c_str());
                        if (Deckastore::get().get_mode() == Deckadence::mode_t::Client) {
                            printf("Submitted %llu's thumbnail request to the queue.\n", this->m_uuid);
                            this->parent_profile.enqueue_thumbnail(this->m_uuid, this->thumbnail.get());
                        } else {
                            (*config)["thumbnail_hash"] = hash_str;
                        }
                    }
                } else {
                    (*config)["thumbnail_hash"] = hash_str;
                }
            } else if (Deckastore::get().get_mode() == Deckadence::mode_t::Client) {
                printf("Submitted %llu's thumbnail request to the queue.\n", this->m_uuid);
                this->parent_profile.enqueue_thumbnail(this->m_uuid, this->thumbnail.get());
            }
        }
    }
}

void Item::draw_properties() {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Label: ");
    ImGui::SameLine();
    // TODO: Multiline label support
    ImGui::InputText("##label", &this->m_label);
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
        this->thumbnail->destroy();
        this->m_thumbnail->destroy();
        this->m_thumbnail = this->thumbnail;
    } else {
        if (this->m_thumbnail != this->thumbnail) {
            fs::create_directories(this->parent_profile.get_profile_dir());
            if (this->m_thumbnail->is_init() || this->m_thumbnail->ready_to_bind())
                this->m_thumbnail->write_png((this->parent_profile.get_profile_dir() / (std::to_string(this->get_uuid()) + ".png")).generic_string().c_str());
        }
        this->thumbnail = this->m_thumbnail;
    }
    (*this->config)["idx"] = idx;
    (*this->config)["type"] = this->parent->items[idx]->get_typename();
    (*this->config)["has_thumbnail"] = this->has_thumbnail;
    (*this->config)["label"] = this->label;
    (*this->config)["uuid"] = this->m_uuid;
    fs::path img_path = this->parent_profile.get_profile_dir() / (std::to_string(this->get_uuid()) + ".png");
    if (fs::exists(img_path)) {
        SHA256 hash;
        std::ifstream reader(img_path, std::ios::binary | std::ios::ate);

        std::streamsize len = reader.tellg();
        reader.seekg(0, std::ios::beg);

        vector<uint8_t> data(len);

        reader.read(reinterpret_cast<char*>(data.data()), len);
        reader.close();

        hash.update(data.data(), len);

        (*this->config)["thumbnail_hash"] = hash.toString(hash.digest());
    } else {
        (*this->config)["has_thumbnail"] = false;
    }
}

Item* Item::copy_properties(Item& other) {
    if (this != &other) {
        this->m_label = other.m_label;
        this->parent = other.parent;
        this->m_has_thumbnail = other.m_has_thumbnail;
        this->m_thumbnail = other.m_thumbnail;
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
    assert(!m_uuid_set);
    this->m_uuid = uuid;
    if (this->parent_profile.register_uuid(uuid)) {
        this->m_uuid = 0;
        return -1;
    }
    m_uuid_set = true;
    (*this->config)["uuid"] = m_uuid;
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
