#include "../../include/Header.hpp"
#include "../../include/Config.hpp"
#include "../../include/GUI/GUI.hpp"
#include "../../include/Server.hpp"

namespace NxSh {

int port = 27015;

bool restart_server = false;

socket_t listen_socket = NX_INVALID_SOCKET;

int server_init() {
    socket_t listen_socket = socket_init();
    if (listen_socket == NX_INVALID_SOCKET) return -1;
    else server_loop(listen_socket);
    nx_sock_close(listen_socket);
    nx_sock_cleanup();
    shells_lock.lock();
    shells.clear();
    shells_lock.unlock();
    if (restart_server) {
        LOG(stdout, "--------------RESTARTING SERVER--------------\n");
        return server_init();
    }
    restart_server = true;
    done = true;
    return 0;
}

socket_t socket_init() {
    // TODO: move to rw_portstore
    string portstore = string(getenv("APPDATA")) + "\\NexusShell\\";
    if (!exists(portstore)) create_directories(portstore);
    portstore += "portstore";
    if (exists(portstore)) {
        ifstream reader(portstore);
        char temp[6];
        reader.getline(temp, 6);
        reader.close();
        string tempstr(temp);
        port = atoi(temp);
        if (!port || port > 65535) {
            LOG(stderr, "Port is invalid. Defaulting to 27015.\n");
            port = 27015;
            ofstream writer(portstore);
            writer << port;
            writer.close();
        }
    }
    return socket_setup();
}

void server_loop(socket_t listen_socket) {
    restart_server = false;

    vector<std::shared_ptr<thread>> sockets;
    socket_t client_socket = NX_INVALID_SOCKET;

    bool valid;
    char* idbuf = new(std::nothrow) char[1024 * 256]; // exceptions bad
    if (!idbuf) {
        LOG(stderr, "Failed to allocate memory for verification buffer.\n");
        exit(-1);
    }
    while (!done && !restart_server) {
        memset(idbuf, 0, 1024 * 256);
        valid = socket_accept(client_socket, listen_socket, idbuf);
        if (!valid) {
            nx_sock_close(client_socket);
            continue;
        }
        Shell shell(idbuf);
        LOG(stdout, "Shell ID: %s\n", shell.get_uuid().c_str());
        shell.client = client_socket;
        if (configure_shell(shell)) continue;

        shells[shell.get_uuid()] = shell;
        sockets.push_back(std::make_unique<thread>(shell_comm_loop, shells[shell.get_uuid()]));
    }
    for (auto& socket : sockets) socket->join();
    delete[] idbuf;
}

int socket_accept(socket_t& client_socket, socket_t listen_socket, char* idbuf) {
    while (client_socket == INVALID_SOCKET && !done && !restart_server) 
        client_socket = accept(listen_socket, NULL, NULL);
    int res = recv(client_socket, idbuf, 19, 0);
    if (res < ID_LENGTH || idbuf == NULL) return false;
    if (shell_validate(idbuf)) return true;
    return false;
}

void shell_comm_loop(Shell& shell) {
    char* buffer = new(std::nothrow) char[1024 * 256];
    if (!buffer) {
        LOG(stderr, "Failed to allocate memory for communication buffer for shell: %s\n", shell.get_uuid().c_str());
        goto failed_alloc;
    }
    do {
        shell.res = recv(shell.client, buffer, 19, 0);
        if (shell.res > 0) {
            shell.lock.lock();
            shell_parse_message((const char**)&buffer, shell);
            shell.lock.unlock();
        }
    } while (!done && !restart_server && shell.send_res > 0 && shell.res > 0);
    delete[] buffer;
failed_alloc:
    nx_sock_close(shell.client);
    // TODO: Change all to LOG() function
    LOG(stdout, "%s disconnected.\n", shell.nickname.c_str());
    shells_lock.lock();
    if (g_shell_uuid == shell.get_uuid()) {
        g_clr_dialog_shown = false;
        g_draw_button_props = false;
        g_draw_shell_props = false;
        g_button_prop_idx = -1;
        g_shell_uuid = "";
    }
    shells.erase(shell.get_uuid());
    shells_lock.unlock();
}

bool shell_validate(string msg) {
    if (msg.length() == ID_LENGTH) {
        try {
            uint64_t converter = stoull(msg);
            string checker = to_string(converter);
            if ((checker.length() == ID_LENGTH))
                for (unordered_map<string, Shell>::iterator it = shells.begin(); 
                     it != shells.end(); ++it)
                    if (it->first == checker) return false;
            return true;
        } catch (...) {}
    }
    return false;
}

void shell_parse_message(const char** msg, Shell& shell) {
    if ((*msg)[0] == SHORTCUT_MAGIC) {
        int pos = atoi(*msg + 2);
        // TODO: when client changes directory, send command to change the root on the server (internally) too

        // TODO: Find button and Button::execute()
    }
}

socket_t socket_setup() {
    socket_t listen_socket;
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    #ifdef _WIN32
        WSADATA wsadata;
        if (WSAStartup(MAKEWORD(2, 2), &wsadata)) {
            LPSTR err_msg;
            DWORD fmt_res = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                          NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&err_msg, 0, NULL);
        // TODO: Log and getlasterror
            LOG(stderr, "WSAStartup failed: %s\n", err_msg);
            LocalFree(err_msg);
            return NX_INVALID_SOCKET;
        }
    #endif

    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == NX_INVALID_SOCKET) {
        nx_sock_cleanup();
        return NX_INVALID_SOCKET;
    }

    if (bind(listen_socket, (sockaddr*)&addr, sizeof(addr)) == NX_SOCKET_ERROR) {
        #ifdef _WIN32
            HANDLE nxsh_window = FindWindowA(NULL, "NexusShell");
            if (nxsh_window != INVALID_HANDLE_VALUE) {
                ShowWindow((HWND)nxsh_window, SW_NORMAL);
                SetFocus((HWND)nxsh_window);
            } else {
                LOG(stderr, "Port %d is already occupied by another program.\n"
                            "\tEdit configuration to use another port.\n", port);
            }
            nx_sock_close(listen_socket);
        #endif
        nx_sock_cleanup();
        return NX_INVALID_SOCKET;
    }
    if (listen(listen_socket, SOMAXCONN) == NX_SOCKET_ERROR) {
        nx_sock_close(listen_socket);
        nx_sock_cleanup();
        return NX_INVALID_SOCKET;
    }
    return listen_socket;
}

}