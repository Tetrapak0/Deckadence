#pragma once

#define ID_LENGTH 18
#define SHORTCUT_MAGIC 0x72

#include "../external/Tetrapak0/NexusSockets.h"

#include <vector>
#include <string>

using std::string;
using std::vector;

class Shell;

extern int port;

extern bool restart_server;

extern socket_t listen_socket;

extern int server_init();

extern int socket_init(socket_t& listen_socket);

extern int socket_setup(socket_t& listen_socket);
extern int socket_accept(socket_t& cilent_socket, socket_t listen_socket, char* idbuf);

extern bool shell_validate(string msg);

extern void server_loop(socket_t listen_socket);
extern void shell_comm_loop(string ID);
extern void shell_parse_message(const char** msg, Shell& shell);
