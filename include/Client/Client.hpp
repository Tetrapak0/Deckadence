#pragma once

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

#include "../../external/jsonhpp/json.hpp"
#include "../../external/Tetrapak0/NexusSockets.h"

#include "../Config/Profile.hpp"

namespace fs = std::filesystem;

using std::mutex;
using std::string;
using std::vector;

using json = nlohmann::json;

class Client {
    friend class Deckastore;

    uint64_t m_uuid;
    string nickname;
    // Note: the following to-do has been scrapped because the nested structure is too complex
    //       to efficiently re-construct on each update. We could limit it to only construct what's needed,
    //       but a bit of extra ram usage has never hurt anyone. Besides, it's not like the configurations are huge.
    //       Sockets can currently receive a maximum of 256KB of configuration per client, which may be increased
    //       when we have a plugin system.
    //       Furthermore, the client is the only one truly holding the config. The rest is just scoped references.
    // OldTodo: Construct JSON from classes, don't store it.
    json config;
    string m_nickname = nickname;
public:
    socket_t socket = NX_INVALID_SOCKET;
    int res = 1;
    int send_res = 1;

    int current_profile = 0;
    vector<shared_ptr<Profile>> profiles;
    vector<string> nav_history;

    mutex lock;
    
    [[nodiscard]] uint64_t get_uuid() const;

    void set_nickname(const string& nickname);
    [[nodiscard]] string get_nickname() const;

    [[nodiscard]] Profile& get_current_profile_ref();

    json& get_config();

    int configure();
    void update_config();

    void draw_properties();

    Client() = delete;
    explicit Client(const uint64_t uuid, const socket_t socket);
    Client(const Client& other) :   m_uuid(other.m_uuid),
                                    nickname(other.nickname),
                                    config(other.config),
                                    socket(other.socket),
                                    current_profile(other.current_profile),
                                    profiles(other.profiles) {}
    explicit Client(const Client* other) :  m_uuid(other->m_uuid),
                                            nickname(other->nickname),
                                            config(other->config),
                                            socket(other->socket),
                                            current_profile(other->current_profile),
                                            profiles(other->profiles) {}
    Client(Client&& other) noexcept :   m_uuid(other.m_uuid),
                                        nickname(std::move(other.nickname)),
                                        config(std::move(other.config)),
                                        socket(other.socket),
                                        current_profile(other.current_profile),
                                        profiles(std::move(other.profiles)) {}
    Client& operator=(const Client& other) {
        if (this != &other) {
            m_uuid = other.m_uuid;
            nickname = other.nickname;
            socket = other.socket;
            config = other.config;
            current_profile = other.current_profile;
            profiles = other.profiles;
        }
        return *this;
    }
};
extern int start_client_sequence();
extern int client_gui_init();
