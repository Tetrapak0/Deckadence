#include "../../include/Config/Config.hpp"
#include "../../include/Config/Deckastore.hpp"
#include "../../include/Client/Client.hpp"
#include "../../include/Client/DiscoveryListenerService.hpp"

#include <thread>
#include <string>
#include <vector>

using std::thread;
using std::string;
using std::vector;

void begin_receiver_loop() {
    Deckastore& dxstore = Deckastore::get();
    dxstore.add_task(tasks::CLIENT);
    const status_t& dxstatus = dxstore.get_status();
    Client& dxclient = dxstore.retrieve_current_client();
    string buf;
    pollfd pole{dxclient.socket, POLLIN, 0};
    do {
        buf.clear();
        buf.resize(1024*256, '\0');
        int res = poll(&pole, 1, 2000);
        if (!res) {
            continue;
        } else if (res == NX_SOCKET_ERROR) {
            break;
        }
        // TODO: Make sure entire message is received.
        dxclient.res = recv(dxclient.socket, buf.data(), buf.capacity(), 0);
        if (dxclient.res == NX_INVALID_SOCKET) {
            dxclient.res = 0;
            break;
        }
        buf[dxclient.res] = '\0';
        vector<string> msg_queue;
        size_t current = 0;
        for (size_t i = 0; i < dxclient.res; ++i) {
            if (buf[i] == '\0') {
                msg_queue.emplace_back(&buf[current]);
                current = i + 1;
            }
        }
        if (current < dxclient.res) {
            msg_queue.emplace_back(&buf[current]);
        }

        for (auto& msg : msg_queue) {
            printf("msg: %s\n", msg.c_str());
            switch (msg[0]) {
                case DX_CONFIG_BYTE: {
                    fs::path dkd_dir = get_cfg_dir();
                    fs::create_directories(dkd_dir);
                    json cfg = json::parse(msg.substr(1, msg.length()-1));
                    std::ofstream writer(dkd_dir / (std::to_string(dxclient.get_uuid()) + ".json"));
                    writer << cfg.dump(4);
                    writer.close();
                    // TODO: pass json to .configure(); overload
                    dxclient.configure();
                    for (auto& nav : dxclient.nav_history)
                        dxclient.get_current_profile_ref().root->items[strtoull(nav.c_str(), nullptr, 10)]->execute();
                    break;
                } case DX_DISCONNECT_BYTE: {
                    dxclient.res = 0;
                    break;
                } case DX_EXECUTE_BYTE: {
                    int idx = strtoull(msg.substr(1, msg.length()-1).c_str(), nullptr, 10);
                    Item* tmp = dxclient.get_current_profile_ref().root->items[idx].get();
                    if (!strcmp(tmp->get_typename(), "Folder")) {
                        tmp->execute();
                        dxclient.nav_history.push_back(std::to_string(idx));
                    } else if (!strcmp(tmp->get_typename(), "Go up")) {
                        tmp->execute();
                        dxclient.nav_history.pop_back();
                    }
                    break;
                } default: {
                    break;
                }
            }
        }
    } while (!static_cast<int>(dxstatus) && dxclient.res > 0 && dxclient.send_res > 0);
    dxclient.res = 1;
    dxclient.send_res = 1;
    nx_sock_close(dxclient.socket);
    dxclient.socket = NX_INVALID_SOCKET;
    dxclient.get_current_profile_ref().root = &dxclient.get_current_profile_ref().items;
    dxclient.nav_history.clear();
    dxstore.remove_task(tasks::CLIENT);
}

void begin_client_loop() {
    Deckastore& dxstore = Deckastore::get();
    const status_t& dxstatus = dxstore.get_status();
    while (dxstatus == status_t::RUNNING) {
        if (dxstore.retrieve_current_client().socket == NX_INVALID_SOCKET) {
            start_listening();
        } else {
            begin_receiver_loop();
        }
    }
}

uint64_t find_uuid() {
    fs::path dkd_dir = get_cfg_dir();
    fs::create_directories(dkd_dir);
    uint64_t uuid = generate_uuid();
    if (fs::exists(dkd_dir / "config.json")) {
        std::ifstream reader(dkd_dir / "config.json");
        json cfg = json::parse(reader);
        reader.close();
        // TODO: JSON type checking
        if (cfg.contains("uuid"))
            return cfg["uuid"].get<uint64_t>();
        std::ofstream writer(dkd_dir / "config.json");
        cfg["uuid"] = uuid;
        writer << cfg.dump(4);
        writer.close();
        return uuid;
    }
    // ffs if this ever runs for anybody...
    std::ofstream writer(dkd_dir / "config.json");
    json cfg;
    cfg["uuid"] = uuid;
    writer << cfg.dump(4);
    writer.close();
    return uuid;
}

int start_client_sequence() {
    Deckastore& dxstore = Deckastore::get();

    const uint64_t uuid = find_uuid();
    dxstore.insert_client(uuid, NX_INVALID_SOCKET);
    dxstore.set_selected_uuid(uuid);

    thread t_client(begin_client_loop);

    client_gui_init();

    t_client.join();

    return 0;
}
