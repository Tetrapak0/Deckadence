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
    string buf(1024*256, '\0');
    pollfd pole{dxclient.socket, POLLIN};
    do {
        int res = poll(&pole, 1, 2000);
        if (!res) {
            continue;
        } else if (res == NX_SOCKET_ERROR) {
            break;
        }
        dxclient.res = recv(dxclient.socket, buf.data(), buf.capacity(), 0);
        if (dxclient.res == NX_INVALID_SOCKET)
            dxclient.res = 0;
        buf[dxclient.res] = '\0';
        switch (buf[0]) {
            case DX_CONFIG_BYTE: {
                printf("%s\n", buf.c_str());
                fs::path dkd_dir = get_cfg_dir();
                fs::create_directories(dkd_dir);
                std::ofstream writer(dkd_dir / (std::to_string(dxclient.get_uuid()) + ".json"));
                json cfg = json::parse(buf.substr(1, dxclient.res-1));
                writer << cfg.dump(4);
                writer.close();
                // TODO: pass json to .configure(); overload
                dxclient.configure();
                break;
            } case DX_DISCONNECT_BYTE: {

                dxclient.res = 0;
                break;
            } default: {
                break;
            }
        }
    } while (!static_cast<int>(dxstatus) && dxclient.res > 0 && dxclient.send_res > 0);
    dxclient.res = 1;
    dxclient.send_res = 1;
    nx_sock_close(dxclient.socket);
    dxclient.socket = NX_INVALID_SOCKET;
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
