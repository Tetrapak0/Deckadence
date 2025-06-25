#include "../include/Config/Deckastore.hpp"
#include "../include/Config/Config.hpp"

#include "../include/Client/Client.hpp"
#include "../include/Server/Server.hpp"

#include "../include/Utilities/SingleInstanceSocket.hpp"

#include <thread>

// TODO: Keep checking if config file has been changed and reload if it has been
// TODO: Installer

int main(int argc, char** argv) {
    setbuf(stdout, nullptr);
    setbuf(stderr, nullptr);
    int ret = 0;
    Deckastore& dxstore = Deckastore::get();
    if (nx_sock_init()) {
    #ifdef _WIN32
        printf("Failed to initialize networking. WSAGetLastError(): %d\n", WSAGetLastError());
    #else
        printf("Failed to initialize networking: %s\n", strerror(errno));
    #endif
        return -1;
    }

    dxstore.set_status(status_t::RESTART);
#ifdef _DEBUG
    bool multi_instance = false;
    Deckadence::mode_t orr_mode = Deckadence::mode_t::SERVER;
    bool override_mode = false;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-M"))
            multi_instance = true;
        if (!strcmp(argv[i], "-S") && !override_mode) {
            orr_mode = Deckadence::mode_t::SERVER;
            override_mode = true;
        } else if (!strcmp(argv[i], "-C") && !override_mode) {
            orr_mode = Deckadence::mode_t::CLIENT;
            override_mode = true;
        }
    }
    if (multi_instance) {
        Deckastore::get().disable_sis();
        dxstore.set_status(status_t::RUNNING);
    }
#endif
    std::thread t_si_ensurer(si_socket_init);
    while (true) {
        dxstore.lock.lock();
        if (dxstore.status != status_t::RESTART) {
            dxstore.lock.unlock();
            break;
        }
        dxstore.lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    while (dxstore.status != status_t::DONE) {
        Deckastore::reset();
        if (check_config()) {
            ret = -1;
            break;
        }
#ifdef _DEBUG
        if (override_mode)
            Deckastore::get().set_mode(orr_mode);
#endif
        // TODO: Should probably implement return value
        dxstore.net_query_interfaces();
        switch (static_cast<int>(dxstore.mode)) {
            case static_cast<int>(Deckadence::mode_t::SERVER): {
                ret = start_server_sequence();
                break;
            } case static_cast<int>(Deckadence::mode_t::CLIENT): {
                ret = start_client_sequence();
                break;
            } default: {
                ret = -1;
                break;
            }
        }
    }
    t_si_ensurer.join();
    nx_sock_cleanup();
    return ret;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {
#ifdef _DEBUG
    AllocConsole();
    AttachConsole(GetCurrentProcessId());

    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);
    freopen("CON", "r", stdin);
#endif
    int ret = main(__argc, __argv);
#ifdef _DEBUG
    FreeConsole();
#endif
    return ret;
}
#endif
