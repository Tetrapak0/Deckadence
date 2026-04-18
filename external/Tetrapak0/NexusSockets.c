#include "NexusSockets.h"

typedef unsigned long long uint64_t;

uint64_t recvn(int socket, void* out, uint64_t len, int flags) {
    uint64_t received = 0;
    while (received < len) {
        uint64_t res = recv(socket, (char*)out+received, len-received, flags);
        if (res < 1) {
            return res;
        }
        received += res;
    }
    return received;
}