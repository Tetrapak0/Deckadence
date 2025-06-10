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
                printf("%ls: %s\n", adapter->FriendlyName, ip);
                printf("%ls\n", adapter->Description);
                interfaces.emplace_back(adapter->AdapterName, ip);
            }
        }
    }
#else
    ifaddrs* ifaddr;
    if (getifaddrs(&ifaddr) == -1) {
        return interfaces;
    }

    for (ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) {
            continue;
        }
        void* addr = &((sockaddr_in*)ifa->ifa_addr)->sin_addr;
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, addr, ip, sizeof(ip));
        if (strcmp(ip, "172.0.0.1"))
            interfaces.emplace_back(ifa->ifa_name, ip);
    }
    freeifaddrs(ifaddr);
#endif
    return interfaces;
}

int start_discovery_service() {
    Deckastore& dxstore = Deckastore::get();
    const status_t& dxstatus = dxstore.get_status();
    dxstore.add_task(tasks::DISCOVERY);
    uint16_t port = dxstore.get_port();
    nx_sock_init();
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

        struct sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(iface.addr.to_string().c_str());
        addr.sin_port = htons(MCAST_PORT);
        struct ip_mreq mreq{};
        mreq.imr_multiaddr.s_addr = inet_addr(MCAST_IP);
        mreq.imr_interface.s_addr = inet_addr(iface.addr.to_string().c_str());

        bind(iface.discovery_sock, (sockaddr*)&addr, sizeof(addr));

        setsockopt(iface.discovery_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq));
    }

    char hn[MAX_HOSTNAME_LEN];
    const char* un = getenv(USR_VAR);
    gethostname(hn, MAX_HOSTNAME_LEN);
    while (!static_cast<int>(dxstatus)) {
        for (auto& iface : interfaces) {
            string msg = string(un) + "@" + hn + " (" + iface.addr.to_string() + ":" + std::to_string(port) + ")";
            sendto(iface.discovery_sock, msg.c_str(), msg.length(), 0, (sockaddr*)&mcast_addr, sizeof(mcast_addr));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    for (auto& iface : interfaces) {
        nx_sock_close(iface.discovery_sock);
    }
    nx_sock_cleanup();
    dxstore.remove_task(tasks::DISCOVERY);
    return 0;
}

