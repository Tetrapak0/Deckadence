#ifndef __NX_SOCK_H__
#define __NX_SOCK_H__

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <afunix.h>
#include <iphlpapi.h>
// #include <Windows.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>
#include <netdb.h>
#include <poll.h>
#include <fcntl.h>

#include <errno.h>
#endif

#ifdef _WIN32
typedef SOCKET socket_t;
typedef int    socklen_t;

#define SHUT_RD   SD_RECEIVE
#define SHUT_WR   SD_SEND
#define SHUT_RDWR SD_BOTH

#define NX_INVALID_SOCKET INVALID_SOCKET

inline int nx_sock_init() {
    WSAData wsadata;
    return WSAStartup(MAKEWORD(2, 2), &wsadata);
}
#define poll(fdarray, fds, timeout) WSAPoll(fdarray, fds, timeout);
#define nx_sock_close(sockfd) closesocket(sockfd)
#define nx_sock_cleanup() WSACleanup()
#else
typedef int socket_t;

#define SD_RECEIVE SHUT_RD
#define SD_SEND    SHUT_WR
#define SD_BOTH    SHUT_RDWR

#define NX_INVALID_SOCKET -1

#define nx_sock_init() (0)
#define nx_sock_close(sockfd) close(sockfd)
#define nx_sock_cleanup() {}
#endif

#define NX_SOCKET_ERROR -1

// shutdown() flags
#define NXS_SHUT_R  SHUT_RD
#define NXS_SHUT_W  SHUT_WR
#define NXS_SHUT_RW SHUT_RDWR

#endif
