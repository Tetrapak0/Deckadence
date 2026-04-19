#pragma once

#include "Client/Client.hpp"
#include "Config/Config.hpp"
#include "Config/ExecutableItem.hpp"

#ifdef __linux__
#pragma push_macro("None")
#undef None
#endif

enum class MessageType : uint32_t {
    None = 0,
    Config = 'C',
    Disconnect = 'D',
    Execute = 'E',
    Length = 'L',
    ThumbnailRequest = 'T',
    ThumbnailDelivery = 'U',
    Wakeup = 'W'
};

#ifdef __linux__
#pragma pop_macro("None")
#endif

class MessageBase {
public:
    virtual void dispatch(Client& dxclient) = 0;
    virtual ~MessageBase() = default;
protected:
    string msg;
    explicit MessageBase(const string& message) : msg(message) {}
};

template <MessageType T>
class Message : MessageBase {
    void dispatch(Client& dxclient) override {}
public:
    Message(const string& msg, Client& dxclient) : MessageBase(msg) {
        Message<T>::dispatch(dxclient);
    }
    ~Message() override = default;
};

template<>
class Message<MessageType::None> final : MessageBase {
    void dispatch(Client& dxclient) override {}
public:
    Message() = delete;
};

template<>
class Message<MessageType::Config> final : MessageBase {
    enum class ConfigType {
        Full = 0,
        Partial = 1
    } config_type;

    void dispatch(Client& dxclient) override {
        dxclient.lock.lock();

        json cfg = json::parse(msg);

        config_type = cfg["config_type"] == "partial" ? ConfigType::Partial : ConfigType::Full;

        cfg.erase("config_type");

        if (config_type == ConfigType::Partial) {
            Profile& dxprofile = dxclient.get_current_profile_ref();
            json& items_ref = (*dxprofile.root->config)["items"];
            if (!cfg.contains("idx")) {
                dxclient.lock.unlock();
                return;
            }

            int idx = cfg["idx"];

            if (cfg.size() == 1) {
                for (int i = 0; i < items_ref.size(); ++i) {
                    if (items_ref[i]["idx"] == idx) {
                        items_ref.erase(i);
                        break;
                    }
                }
                dxprofile.root->items[idx] = std::make_shared<ExecutableItem>(nullptr, dxprofile, dxprofile.root);
            } else {
                int item_idx_in_array = -1;
                for (int i = 0; i < items_ref.size(); ++i) {
                    if (items_ref[i]["idx"] == idx) {
                        items_ref[i] = cfg;
                        item_idx_in_array = i;
                        break;
                    }
                }
                if (item_idx_in_array > -1) {
                    dxprofile.root->items[idx] = std::shared_ptr<Item>(Deckastore::get().type_registry.retrieve_type(cfg["type"].get<string>().c_str(), &items_ref[item_idx_in_array], dxprofile, dxprofile.root));
                } else {
                    items_ref.emplace_back(cfg);
                    dxprofile.root->items[idx] = std::shared_ptr<Item>(Deckastore::get().type_registry.retrieve_type(cfg["type"].get<string>().c_str(), &items_ref.back(), dxprofile, dxprofile.root));
                }
            }
        }

        dxclient.write_config();

        dxclient.lock.unlock();

        // TODO: pass json to .configure(); overload
        if (config_type == ConfigType::Full)
            dxclient.configure();
    }
public:
    explicit Message(const string& msg, Client& dxclient) : MessageBase(msg) {
        dispatch(dxclient);
    }
};

template<>
class Message<MessageType::Disconnect> final : MessageBase {
    void dispatch(Client& dxclient) override {
        dxclient.res = 0;
    }
public:
    explicit Message(const string& msg, Client& dxclient) : MessageBase(msg) {
        dispatch(dxclient);
    }
};

template<>
class Message<MessageType::Execute> final : MessageBase {
    void dispatch(Client& dxclient) override {
        uint8_t idx = msg[0];
        shared_ptr item = dxclient.get_current_profile_ref().root->items[idx];
        if (Deckastore::get().get_mode() == Deckadence::mode_t::Client) {
            if (item->openable_in_editor()) {
                item->execute();
            }
        } else {
            item->execute();
        }
    }
public:
    explicit Message(const string& msg, Client& dxclient) : MessageBase(msg) {
        dispatch(dxclient);
    }
};

template<>
class Message<MessageType::ThumbnailRequest> final : MessageBase {
    void dispatch(Client& dxclient) override {
        uint64_t item_uuid = 0;
        memcpy(&item_uuid, msg.data(), sizeof(uint64_t));
        printf("Received thumbnail request from %llu for %llu\n", dxclient.get_uuid(), item_uuid);
        string path = (get_cfg_dir() / std::to_string(dxclient.get_uuid()) / dxclient.get_current_profile_ref().name / (std::to_string(item_uuid) + ".png")).generic_string();

        vector<uint8_t> data;

        if (fs::exists(path)) {
            std::ifstream reader(path, std::ios::binary | std::ios::ate);

            std::streamsize len = reader.tellg();
            reader.seekg(0, std::ios::beg);

            data.reserve(len);

            reader.read(reinterpret_cast<char*>(data.data()), len);
            reader.close();
        } else {
            printf("File not found, discarding.\n");
            data.reserve(3);
            memcpy(data.data(), "NUL", 3);
        }

        string msg;
        msg.reserve(2 * sizeof(uint64_t) + data.size());
        uint64_t header = construct_header(sizeof(uint64_t) + data.size(), static_cast<uint32_t>(MessageType::ThumbnailDelivery));
        msg.append(reinterpret_cast<const char*>(&header), sizeof(uint64_t));
        msg.append(this->msg);
        msg.append(reinterpret_cast<const char*>(data.data()), data.size());

        dxclient.send_res = send(dxclient.socket, msg.data(), msg.length(), 0);
    }
public:
    explicit Message(const string& msg, Client& dxclient) : MessageBase(msg) {
        dispatch(dxclient);
    }
};

template<>
class Message<MessageType::ThumbnailDelivery> final : MessageBase {
    void dispatch(Client& dxclient) override {  
        dxclient.get_current_profile_ref().handle_thumbnail_delivery(msg);
    }
public:
    explicit Message(const string& msg, Client& dxclient) : MessageBase(msg) {
        dispatch(dxclient);
    }
};