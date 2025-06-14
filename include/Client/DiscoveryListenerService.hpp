#pragma once

#include "../../external/Tetrapak0/NexusSockets.h"

#include <mutex>
#include <vector>

using std::mutex;
using std::vector;

struct ServerInfo {
    string hostname;
    string username;
    string ipv4;
    uint16_t port;
    uint64_t last_ping;
    string full;
};
/*
struct ServerInfo {
    IPv4 ip;
    uint16_t port;
    char username[256];
    char hostname[255];
    uint64_t last_ping;
    bool operator==(const ServerInfo& other) const {
        return ip.to_string() == other.ip.to_string() &&
               port == other.port && !strcmp(hostname, other.hostname);
    }
    inline string to_string() {
        return string(username) + "@" + string(hostname) +
               " (" + ip.to_string() + ':' + std::to_string(port) + ")";
    }
};
*/

extern void start_listening();

extern mutex servers_lock;
extern vector<ServerInfo> servers;
extern int selected_server;
