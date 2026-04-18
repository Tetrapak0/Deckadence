#include "../../include/Server/DiscoveryService.hpp"
#include "../../include/Config/Config.hpp"
#include "../../include/Config/Deckastore.hpp"
#include "../../include/Server/Server.hpp"

#include <thread>
#include <utility>
#include <vector>

using std::vector;

#include <stdio.h>
#include <stdlib.h>

vector<NetworkInterface> query_interfaces() {
    vector<NetworkInterface> interfaces;
#ifdef _WIN32
    ULONG buflen = 15 * 1024;
    vector<char> buffer(buflen);
    auto* adapters = (IP_ADAPTER_ADDRESSES*)buffer.data();
    if (!GetAdaptersAddresses(AF_INET, NULL, nullptr, adapters, &buflen)) {
        for (auto* adapter = adapters; adapter; adapter = adapter->Next) {
            if (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK || adapter->IfType == IF_TYPE_TUNNEL || adapter->OperStatus != IfOperStatusUp)
                continue;
            for (auto* unicast_addr = adapter->FirstUnicastAddress; unicast_addr; unicast_addr = unicast_addr->Next) {
                auto* saddr = (sockaddr_in*)unicast_addr->Address.lpSockaddr;
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &saddr->sin_addr, ip, sizeof(ip));
                interfaces.emplace_back(adapter->AdapterName, ip);
            }
        }
    }
#else
    ifaddrs* ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1) {
        return interfaces;
    }

    for (ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET)
            continue;
        if (ifa->ifa_flags & IFF_LOOPBACK)
            continue;
        if (!(ifa->ifa_flags & IFF_UP))
            continue;
        if (!strncmp(ifa->ifa_name, "tun", 3) || !strncmp(ifa->ifa_name, "tap", 3) ||
            !strncmp(ifa->ifa_name, "p2p", 3) || !strncmp(ifa->ifa_name, "wifi-direct", 11))
            continue;
        void* addr = &((sockaddr_in*)ifa->ifa_addr)->sin_addr;
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, addr, ip, sizeof(ip));
        interfaces.emplace_back(ifa->ifa_name, ip);
    }
    freeifaddrs(ifaddr);
#endif
    return interfaces;
}

int start_discovery_service() {
    Deckastore& dxstore = Deckastore::get();
    const status_t& dxstatus = dxstore.get_status();
    const bool& discoverable = dxstore.get_discoverable();
    dxstore.add_task(tasks::Discovery);
    uint16_t port = dxstore.get_port();
    vector<NetworkInterface>& interfaces = dxstore.get_ifaces_ref();

    struct sockaddr_in mcast_addr{};
    mcast_addr.sin_family = AF_INET;
    mcast_addr.sin_addr.s_addr = inet_addr(MCAST_IP);
    mcast_addr.sin_port = htons(MCAST_PORT);

    for (auto& iface : interfaces) {
        iface.discovery_sock = socket(AF_INET, SOCK_DGRAM, 0);
    #ifdef _DEBUG // Allow Multi instance
        int reuse = 1;
        setsockopt(iface.discovery_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));
    #endif

        struct sockaddr_in loc_addr{};
        loc_addr.sin_family = AF_INET;
        loc_addr.sin_addr.s_addr = inet_addr(iface.ipv4.to_string().c_str());
        loc_addr.sin_port = htons(MCAST_PORT);
        struct ip_mreq mreq{};
        mreq.imr_multiaddr.s_addr = inet_addr(MCAST_IP);
        mreq.imr_interface.s_addr = inet_addr(iface.ipv4.to_string().c_str());

        bind(iface.discovery_sock, (sockaddr*)&loc_addr, sizeof(loc_addr));

        setsockopt(iface.discovery_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq));
    }

    char hn[253]; // max hostname on Linux
    const char* un = getenv(USR_VAR);
    gethostname(hn, 253);
    while (!static_cast<int>(dxstatus) && discoverable) {
        for (auto& iface : interfaces) {
            string msg = string(un) + "@" + hn + " (" + iface.ipv4.to_string() + ":" + std::to_string(port) + ")";
            sendto(iface.discovery_sock, msg.c_str(), msg.length(), 0, (sockaddr*)&mcast_addr, sizeof(mcast_addr));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    for (auto& iface : interfaces) {
        nx_sock_close(iface.discovery_sock);
    }
    dxstore.remove_task(tasks::Discovery);
    return 0;
}

