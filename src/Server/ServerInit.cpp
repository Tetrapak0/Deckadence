#include "Server/Server.hpp"
#include "Client/Client.hpp"
#include "Config/Config.hpp"
#include "Config/Deckastore.hpp"
#include "Server/Message.hpp"

#include <thread>

using std::thread;
using std::vector;

int begin_comm_loop(uint64_t uuid) {
    Deckastore& dxstore = Deckastore::get();
    Client& client = dxstore.retrieve_client(uuid);
    client.update_config();
    const status_t& dxstatus = dxstore.get_status();
    pollfd pole{client.socket, POLLIN};
    do {
        int res = poll(&pole, 1, 2000);
        pole.revents = 0;
        if (!res)
            continue;
        if (res < 0)
            break;

        if (client.get_nickname().empty())
            printf("Received message from %llu\n", client.get_uuid());
        else
            printf("Received message from %s\n", client.get_nickname().c_str());

        uint64_t header = 0;

        client.res = recvn(client.socket, reinterpret_cast<char*>(&header), sizeof(uint64_t), NULL);
        if (client.res <= 0) {
#ifdef _WIN32
            printf("%d\n", WSAGetLastError());
#else
            printf("%s\n", strerror(errno));
#endif
            break;
        }

        uint32_t len = ntohl(header >> 32);
        uint32_t type = ntohl(header & 0xFFFFFFFF);

        printf("Message size: %lu, type: %lu\n", len, type);

        string buf(len, '\0');

        client.res = recvn(client.socket, buf.data(), len, NULL);
        if (client.res <= 0) {
#ifdef _WIN32
            printf("%d\n", WSAGetLastError());
#else
            printf("%s\n", strerror(errno));
#endif
            break;
        }

        client.lock.lock();
        switch (static_cast<MessageType>(type)) {
            case MessageType::Execute: {
                (void)Message<MessageType::Execute>(buf, client);
                break;
            } case MessageType::ThumbnailRequest: {
                (void)Message<MessageType::ThumbnailRequest>(buf, client);
                break;
            }
        }
        client.lock.unlock();
    } while (!static_cast<int>(dxstatus) && client.res > 0 && client.send_res > 0);

    dxstore.disconnect_client(client.get_uuid(), "Server closed.");
    return 0;
}

bool check_client_uuid(string buf) {
    for (int i = 0; i < buf.length(); ++i) {
        if (isdigit(buf[i]))
            continue;
        return false;
    }
    if (strtoull(buf.c_str(), nullptr, 10) < 1)
        return false;
    return true;
}

socket_t accept_client(socket_t sock, uint64_t& uuid) {
    Deckastore& dxstore = Deckastore::get();
    const socket_t client_socket = accept(sock, nullptr, nullptr); // TODO: Correspond the client with an IP
    if (client_socket == NX_INVALID_SOCKET) {
        fprintf(stderr, "Failed to accept client.\n");
        uuid = DX_INVALID_UUID;
        return NX_INVALID_SOCKET;
    }
    char buf[21];
    if (int res = recv(client_socket, buf, 21, NULL); res > 0) {
        buf[res-1] = '\0'; // null terminator is counted too
        uuid = strtoull(buf, nullptr, 10);
        // TODO: client_exists should probably be handled in insert_client
        if (check_client_uuid(buf) && !dxstore.client_exists(uuid)) {
            printf("%llu connected.\n", uuid);
            return client_socket;
        }
    }
    nx_sock_close(client_socket);
    uuid = DX_INVALID_UUID;
    return NX_INVALID_SOCKET;
}

int begin_listen_loop() {
    Deckastore& dxstore = Deckastore::get();
    dxstore.add_task(tasks::Server);
    vector<NetworkInterface>& interfaces = dxstore.get_ifaces_ref();

    vector<std::unique_ptr<thread>> client_threads;

    vector<pollfd> pollees(interfaces.size());
    for (int i = 0; i < interfaces.size(); ++i) {
        pollees[i].events = POLLIN;
        pollees[i].fd = interfaces[i].listen_socket;
    }

    while (!static_cast<int>(dxstore.get_status())) {
        const int res = poll(pollees.data(), interfaces.size(), 2000);
        if (!res)
            continue;
        if (res == NX_SOCKET_ERROR)
            break;
        printf("Connection request(s) received.\n");
        for (auto& p : pollees) {
            if (p.revents) {
                uint64_t uuid = DX_INVALID_UUID;
                socket_t sock = accept_client(p.fd, uuid);
                if (sock == NX_INVALID_SOCKET || uuid == DX_INVALID_UUID)
                    continue;
                dxstore.insert_client(uuid, sock);
                client_threads.push_back(std::make_unique<thread>(begin_comm_loop, uuid));
                p.revents = 0;
            }
        }
    }
    for (auto& client : client_threads)
        client->join();
    for (auto& iface : interfaces) {
        if (iface.listen_socket != NX_INVALID_SOCKET)
            nx_sock_close(iface.listen_socket);
    }
    dxstore.remove_task(tasks::Server);
    return 0;
}

int start_server_sequence() {
    Deckastore& dxstore = Deckastore::get();
    vector<NetworkInterface>& interfaces = dxstore.get_ifaces_ref();
    for (auto& iface : interfaces) {
        iface.listen_socket = create_socket(iface);
        if (iface.listen_socket == NX_INVALID_SOCKET) {
            fprintf(stderr, "Invalid socket was returned.\n");
            dxstore.remove_task(tasks::Server);
            dxstore.set_status(status_t::Done);
            return -1;
        }
    }
    thread t_server(begin_listen_loop);
    thread t_discovery([&discovery = Deckastore::get().get_discoverable()](){
        Deckastore& dxstore = Deckastore::get();
        // Allows discovery to be toggled without a restart
        while (dxstore.get_status() == status_t::Running) {
            if (discovery) start_discovery_service();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });

    server_gui_init();
 
    t_discovery.join();
    t_server.join();

    return 0;
}
