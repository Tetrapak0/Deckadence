#pragma once

#include "../../external/Tetrapak0/NexusSockets.h"

#define MCAST_IP   "239.69.69.69" // 239.0.0.0 - 239.255.255.255
#define MCAST_PORT 38489
//username@hostname (ip:port)+'\0'
#define BUFSIZE (256+1+255+2+15+1+5+1+1) // 537

#include <cassert>
#include <string>
#include <vector>
using std::string;
using std::vector;

struct IPv4 {
    uint8_t byte1 = 0;
    uint8_t byte2 = 0;
    uint8_t byte3 = 0;
    uint8_t byte4 = 0;
    string to_string() const {
        return std::to_string(byte1) + '.' +
               std::to_string(byte2) + '.' +
               std::to_string(byte3) + '.' +
               std::to_string(byte4);
    }
    void from_string(const string& ipv4) {
        assert(ipv4.length() <= 15);
        int periodpos = 0;
        int bytes_set = 0;
        for (int i = 0; i <= ipv4.length(); ++i) {
            if (isdigit(ipv4[i])) continue;
            if (!bytes_set)
                byte1 = strtol(ipv4.substr(0, i).c_str(), nullptr, 10);
            else if (bytes_set == 1)
                byte2 = strtol(ipv4.substr(periodpos + 1, i).c_str(), nullptr, 10);
            else if (bytes_set == 2)
                byte3 = strtol(ipv4.substr(periodpos + 1, i).c_str(), nullptr, 10);
            else if (bytes_set == 3)
                byte4 = strtol(ipv4.substr(periodpos + 1, i).c_str(), nullptr, 10);
            periodpos = i;
            ++bytes_set;
        }
    }
    IPv4() = default;
    explicit IPv4(const string& ipv4) {
        from_string(ipv4);
    }
};

struct NetworkInterface {
    string name;
    IPv4 addr{};
    socket_t discovery_sock = NX_INVALID_SOCKET;
    socket_t listen_socket = NX_INVALID_SOCKET;
    NetworkInterface(string name, const char* addr) : name(std::move(name)), addr(addr) {}
    NetworkInterface(string name, const IPv4& addr) : name(std::move(name)), addr(addr) {}
    NetworkInterface() = default;
};

vector<NetworkInterface> query_interfaces();
