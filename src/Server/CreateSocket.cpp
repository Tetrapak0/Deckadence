#include "../../include/Server/Server.hpp"
#include "../../include/Config/Deckastore.hpp"

#include <stdio.h>

socket_t create_socket(NetworkInterface iface) {
    int res = 0;
    if (nx_sock_init()) {
        fprintf(stderr, "Server initialization failed.\n");
        return NX_INVALID_SOCKET;
    }

    socket_t listen_socket = NULL;

    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == NX_INVALID_SOCKET) {
        fprintf(stderr, "socket() failed.\n");
        nx_sock_cleanup(); // Winsock works by counting WSAStartup calls and equaling them to Cleanup calls.
        // TODO: Maybe only have one WSAStartup/Cleanup call
        return NX_INVALID_SOCKET;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(Deckastore::get().get_port());
    addr.sin_addr.s_addr = inet_addr(iface.addr.to_string().c_str());

    res = bind(listen_socket, (sockaddr*)&addr, sizeof(addr));
    if (res == NX_SOCKET_ERROR) {
        fprintf(stderr, "bind() failed.\n");
        fprintf(stderr, "%d\n", WSAGetLastError());
        nx_sock_cleanup();
        return NX_INVALID_SOCKET;
    }

    res = listen(listen_socket, SOMAXCONN);
    if (res == NX_SOCKET_ERROR) {
        fprintf(stderr, "listen() failed.\n");
        nx_sock_cleanup();
        return NX_INVALID_SOCKET;
    }

    return listen_socket;
}
