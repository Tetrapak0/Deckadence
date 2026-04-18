#pragma once

#include "../../external/Tetrapak0/NexusSockets.h"
#include "DiscoveryService.hpp"

int start_server_sequence();
socket_t create_socket(NetworkInterface iface);
int start_discovery_service();
int server_gui_init();
