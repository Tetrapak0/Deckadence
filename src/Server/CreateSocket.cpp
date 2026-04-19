#include "Server/Server.hpp"
#include "Config/Deckastore.hpp"

#include <stdio.h>

socket_t create_socket(NetworkInterface iface) {
    int res = 0;

    socket_t listen_socket = NX_INVALID_SOCKET;

    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == NX_INVALID_SOCKET) {
        fprintf(stderr, "socket() failed.\n");
        return NX_INVALID_SOCKET;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(Deckastore::get().get_port());
    addr.sin_addr.s_addr = inet_addr(iface.ipv4.to_string().c_str());

    res = bind(listen_socket, (sockaddr*)&addr, sizeof(addr));
    if (res == NX_SOCKET_ERROR) {
        fprintf(stderr, "bind() failed.\n");
        return NX_INVALID_SOCKET;
    }

    res = listen(listen_socket, SOMAXCONN);
    if (res == NX_SOCKET_ERROR) {
        fprintf(stderr, "listen() failed.\n");
        return NX_INVALID_SOCKET;
    }

    return listen_socket;
}
