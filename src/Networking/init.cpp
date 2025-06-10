#include "../../include/Header.hpp"
#include "../../include/Config.hpp"
#include "../../include/Server.hpp"

int initialize_discovery_announcer() {

}

int initialize_networking() {
    nx_sock_init();
    thread announcer(initialize_discovery_announcer);
    while (!restart_server) server_init(); // TODO: Status reporting
    // TODO: Properly shutdown announcer
    return 0;
}