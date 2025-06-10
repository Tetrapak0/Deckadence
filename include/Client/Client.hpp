#pragma once

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

#include "../../external/jsonhpp/json.hpp"
#include "../../external/Tetrapak0/NexusSockets.h"

#include "Profile.hpp"

namespace fs = std::filesystem;

using std::mutex;
using std::string;
using std::vector;

using json = nlohmann::json;

class Client {
    uint64_t m_uuid;
    string nickname;
    json config;
    string m_nickname = nickname;
public:
    socket_t socket = NX_INVALID_SOCKET;
    int res = 1;
    int send_res = 1;

    int current_profile = 0;
    vector<Profile> profiles;

    mutex lock;
    
    [[nodiscard]] uint64_t get_uuid() const;

    void set_nickname(const string& nickname);
    [[nodiscard]] string get_nickname() const;

    [[nodiscard]] Profile& get_current_profile_ref();

    json& get_config();

    int configure();

    void draw_properties();

    Client() = delete;
    explicit Client(const uint64_t uuid, const socket_t socket) : m_uuid(uuid), socket(socket) {
        if (configure()) {
            if (this->socket != NX_INVALID_SOCKET) {
                nx_sock_close(this->socket);
                this->socket = NX_INVALID_SOCKET;
            }
        }
        // TODO: if client has config and server doesn't, show a dialog and send config to client
    }
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
