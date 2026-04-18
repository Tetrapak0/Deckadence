#include "Config/Deckastore.hpp"
#include "Config/Config.hpp"
#include "Client/Client.hpp"
#include "Client/DiscoveryListenerService.hpp"
#include "Server/Message.hpp"

#include <thread>
#include <string>
#include <vector>

using std::thread;
using std::string;
using std::vector;

void begin_receiver_loop() {
    Deckastore& dxstore = Deckastore::get();
    dxstore.add_task(tasks::Client);
    const status_t& dxstatus = dxstore.get_status();
    Client& dxclient = dxstore.retrieve_current_client();

    pollfd pole{dxclient.socket, POLLIN, 0};
    do {
        int res = poll(&pole, 1, 2000);
        if (!res)
            continue;
        if (res == NX_SOCKET_ERROR)
            break;

        uint64_t header = 0;
        dxclient.res = recvn(dxclient.socket, &header, sizeof(uint64_t), 0);
        if (dxclient.res <= 0) {
#ifdef _WIN32
            printf("%d\n", WSAGetLastError());
#else
            printf("%s\n", strerror(errno));
#endif
            break;
        }

        printf("%llu\n", header);

        header = ntohll(header);
        uint32_t len  = header >> 32;
        uint32_t type = header & 0xFFFFFFFF;

        printf("%llu\n", header);

        printf("size: %lu, type: %lu\n", len, type);

        string buf(len, '\0');

        dxclient.res = recvn(dxclient.socket, buf.data(), len, 0);
        if (dxclient.res <= 0) {
#ifdef _WIN32
            printf("%d\n", WSAGetLastError());
#else
            printf("%s\n", strerror(errno));
#endif
            break;
        }

        // Thanks for runtime template dispatch, C++ committee
        switch (static_cast<MessageType>(type)) {
            case MessageType::Config: {
                (void)Message<MessageType::Config>(buf, dxclient);
                break;
            } case MessageType::Disconnect: {
                (void)Message<MessageType::Disconnect>(buf, dxclient);
                break;
            } case MessageType::Execute: {
                (void)Message<MessageType::Execute>(buf, dxclient);
                break;
            } case MessageType::ThumbnailDelivery: {
                (void)Message<MessageType::ThumbnailDelivery>(buf, dxclient);
                break;
            } default: {
                // Look up plugin message types when we add plugin support.
                // Pass to plugin's master event dispatch because no runtime
                // template dispatch
            }
        }
    } while (!static_cast<int>(dxstatus) && dxclient.res > 0 && dxclient.send_res > 0);
    dxclient.res = 1;
    dxclient.send_res = 1;
    nx_sock_close(dxclient.socket);
    dxclient.socket = NX_INVALID_SOCKET;
    dxclient.get_current_profile_ref().root = &dxclient.get_current_profile_ref().items;
    dxstore.remove_task(tasks::Client);
}

void begin_client_loop() {
    Deckastore& dxstore = Deckastore::get();
    const status_t& dxstatus = dxstore.get_status();
    while (dxstatus == status_t::Running) {
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
