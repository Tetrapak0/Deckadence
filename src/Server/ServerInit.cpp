#include "../../include/Server/Server.hpp"
#include "../../include/Client/Client.hpp"
#include "../../include/Config/Config.hpp"
#include "../../include/Config/Deckastore.hpp"

#include <thread>

using std::thread;
using std::vector;

int parse_message(const Client& client, const string& msg) {
    printf("msg: %s; ", msg.c_str());
    if (msg[0] == DX_EXECUTE_BYTE) {
        for (int i = 1; i < msg.length(); ++i) {
            if (isdigit(msg[i])) continue;
            return -1;
        }
        printf("%llu\n", strtoull(msg.substr(1, msg.length()-1).c_str(), nullptr, 10));
        client.profiles[client.current_profile].items[strtoull(msg.substr(1, msg.length()-1).c_str(), nullptr, 10)].execute();
    }
    return 0;
}

int begin_comm_loop(uint64_t uuid) {
    Deckastore& dxstore = Deckastore::get();
    Client& client = dxstore.retrieve_client(uuid);
    const status_t& dxstatus = dxstore.get_status();
    char buf[1024] = {0};
    {
        // TODO: Truncate commands and types from message
        string msg = string(1, DX_CONFIG_BYTE) + client.get_config().dump();
        send(client.socket, msg.c_str(), msg.length(), 0);
    }

    pollfd pole{client.socket, POLLIN};
    do {
        int res = poll(&pole, 1, 2000);
        pole.revents = 0;
        switch (res) {
            case 0: {
            } case NX_SOCKET_ERROR: {
                break;
            } default: {
                printf("Received message from %llu\n", client.get_uuid());
                // TODO: change length of recv
                client.res = recv(client.socket, buf, 1024, NULL);
                if (client.res > 0) {
                    client.lock.lock();
                    parse_message(client, buf);
                    client.lock.unlock();
                }
            }
        }
    } while (!static_cast<int>(dxstatus) && client.res > 0 && client.send_res > 0);

    string reason;
    if (static_cast<int>(dxstatus))
        reason = "Server closed.";
    dxstore.disconnect_client(client.get_uuid(), reason);
    printf("%llu disconnected.\n", client.get_uuid());
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

Client accept_client(socket_t sock) {
    Deckastore& dxstore = Deckastore::get();
    const socket_t client_socket = accept(sock, nullptr, nullptr); // TODO: Correspond the client with an IP
    if (client_socket == NX_INVALID_SOCKET) {
        fprintf(stderr, "Failed to accept client.\n");
        return Client(NULL, client_socket);
    }
    char buf[20];
    if (int res = recv(client_socket, buf, 20, NULL); res > 0) {
        // TODO: Implement a blacklist feature
        buf[res] = '\0';
        uint64_t uuid = strtoull(buf, nullptr, 10);
        if (check_client_uuid(buf) && !dxstore.client_exists(uuid)) {
            Client nclient(uuid, client_socket);
            printf("%llu connected.\n", nclient.get_uuid());
            return nclient;
        }
    }
    // TODO: After multiple failures abort program
    nx_sock_close(client_socket);
    Client nclient(NULL, NX_INVALID_SOCKET);
    return nclient;
}

int begin_listen_loop() {
    Deckastore& dxstore = Deckastore::get();
    dxstore.add_task(tasks::SERVER);
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
                Client nclient(accept_client(p.fd));
                if (nclient.socket == NX_INVALID_SOCKET)
                    continue;
                dxstore.insert_client(nclient);
                client_threads.push_back(std::make_unique<thread>(begin_comm_loop, nclient.get_uuid()));
                p.revents = 0;
            }
        }
    }
    for (auto& client : client_threads)
        client->join();
    for (auto& iface : interfaces) {
        nx_sock_close(iface.listen_socket);
    }
    nx_sock_cleanup();
    dxstore.remove_task(tasks::SERVER);
    return 0;
}

int start_server_sequence() {
    Deckastore& dxstore = Deckastore::get();
    vector<NetworkInterface>& interfaces = dxstore.get_ifaces_ref();
    for (auto& iface : interfaces) {
        iface.listen_socket = create_socket(iface);
        if (iface.listen_socket == NX_INVALID_SOCKET) {
            fprintf(stderr, "Invalid socket was returned.\n");
            dxstore.remove_task(tasks::SERVER);
            dxstore.set_status(status_t::DONE);
            return -1;
        }
    }
    thread t_server(begin_listen_loop);
    thread t_discovery([&discovery = Deckastore::get().get_discoverable()](){
        Deckastore& dxstore = Deckastore::get();
        // Allows discovery to be toggled without a restart
        while (dxstore.get_status() == status_t::RUNNING) {
            if (discovery) start_discovery_service();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });

    server_gui_init();
 
    t_discovery.join();
    t_server.join();

    return 0;
}
