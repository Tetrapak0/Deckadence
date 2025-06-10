#pragma once

#include "../../external/Tetrapak0/NexusSockets.h"
#include "DiscoveryService.hpp"

extern int start_server_sequence();
extern socket_t create_socket(NetworkInterface iface);
extern int start_discovery_service();
extern void server_gui_init();
