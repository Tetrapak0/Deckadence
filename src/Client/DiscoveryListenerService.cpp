#include "../../include/Server/DiscoveryService.hpp"
#include "../../include/Client/DiscoveryListenerService.hpp"
#include "../../include/Config/Deckastore.hpp"

mutex servers_lock;

vector<ServerInfo> servers;
int selected_server = -1;

void transcribe_server(string msg, ServerInfo& si) {
    size_t at = std::find(msg.begin(), msg.end(), '@') - msg.begin();
    size_t sp = std::find(msg.begin(), msg.end(), '(') - msg.begin();
    size_t cl = std::find(msg.begin(), msg.end(), ':') - msg.begin();
    si.username = msg.substr(0, at);
    si.hostname = msg.substr(at+1, sp-at-2);
    si.ipv4     = msg.substr(sp+1, cl-sp-1);
    si.port     = strtol(msg.substr(cl+1, msg.length()-cl-2).c_str(), nullptr, 10);
}
void print_server(const ServerInfo& si) {
    printf("Username:   %s\n", si.username.c_str());
    printf("Hostname:   %s\n", si.hostname.c_str());
    printf("IP address: %s\n", si.ipv4.c_str());
    printf("Port:       %d\n", si.port);
}

void start_listening() {
    Deckastore& dxstore = Deckastore::get();
    dxstore.add_task(tasks::DISCOVERY);
    const status_t& dxstatus = dxstore.get_status();
    Client& dxclient = dxstore.retrieve_current_client();

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
    #ifndef _WIN32
        setsockopt(iface.discovery_sock, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse));
    #endif
#endif

        struct sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(MCAST_PORT);
        struct ip_mreq mreq{};
        mreq.imr_multiaddr.s_addr = inet_addr(MCAST_IP);
        mreq.imr_interface.s_addr = inet_addr(iface.addr.to_string().c_str());

        bind(iface.discovery_sock, (sockaddr*)&addr, sizeof(addr));
        // int loop = 0;
        // setsockopt(iface.sock, IPPROTO_IP, IP_MULTICAST_LOOP, (const char*)&loop, sizeof(loop));
        setsockopt(iface.discovery_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq));
    }
    char buf[BUFSIZE];
    vector<pollfd> pollees(interfaces.size());
    for (int i = 0; i < interfaces.size(); ++i) {
        pollees[i].events = POLLIN;
        pollees[i].fd = interfaces[i].discovery_sock;
    }
    while (dxclient.socket == NX_INVALID_SOCKET && dxstatus == status_t::RUNNING) {
        int pres = poll(pollees.data(), interfaces.size(), 2000);
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if (!pres) {
        } else if (pres == NX_SOCKET_ERROR) {
            // TODO: error checking
            break;
        } else {
            // TODO: we probably don't need sockets in the interfaces list
            for (auto& p : pollees) {
                if (p.revents) {
                    socklen_t addrlen = sizeof(mcast_addr);
                    int len = recvfrom(p.fd, buf, BUFSIZE, 0, (struct sockaddr*)&mcast_addr, &addrlen);
                    if (len == NX_SOCKET_ERROR)
                        len = 0;
                    buf[len] = '\0';
                    auto it = std::find_if(servers.begin(), servers.end(), [&buf](const ServerInfo& si){return si.full == buf;});
                    if (it != servers.end()) {
                        it->last_ping = now;
                    } else {
                        servers_lock.lock();
                        printf("New server found: %s\n", buf);
                        ServerInfo si;
                        si.full = buf;
                        si.last_ping = now;
                        transcribe_server(buf, si);
                        servers.push_back(si);
                        servers_lock.unlock();
                    }
                    p.revents = 0;
                }
            }
        }

        if (!servers.empty()) {
            for (int i = 0; i < servers.size(); ++i) {
                if (now - servers[i].last_ping > 5) {
                    servers_lock.lock();
                    if (selected_server == i) {
                        selected_server = -1;
                    }
                    printf("%s timed out.\n", servers[i].full.c_str());
                    servers.erase(servers.begin() + i);
                    servers_lock.unlock();
                }
            }
        }
    }

    for (auto& iface : interfaces) {
        nx_sock_close(iface.discovery_sock);
    }
    nx_sock_cleanup();
    servers_lock.lock();
    servers.clear();
    servers_lock.unlock();
    dxstore.remove_task(tasks::DISCOVERY);
}
