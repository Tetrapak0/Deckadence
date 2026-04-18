#include "Utilities/SingleInstanceSocket.hpp"
#include "Server/Message.hpp"
#include "Config/Deckastore.hpp"

const char wakeywakeybyte[1] = {static_cast<char>(MessageType::Wakeup)};

// TODO: Add GetLastError & errno everywhere

int si_summon_instance() {
    Deckastore& dxstore = Deckastore::get();
    socket_t waker_upper = socket(AF_INET, SOCK_STREAM, 0);
    if (waker_upper == NX_INVALID_SOCKET) {
        fprintf(stderr, "Failed to create wakeup socket.\n");
        dxstore.set_status(status_t::Done);
        return NX_SOCKET_ERROR;
    }

    sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(38674);

    if (!connect(waker_upper, (sockaddr*)&addr, sizeof(addr))) {
        if (send(waker_upper, wakeywakeybyte, 1, 0) == NX_SOCKET_ERROR) {
            fprintf(stderr, "Failed to wake up main instance.\n");
            dxstore.set_status(status_t::Done);
            nx_sock_close(waker_upper);
            return NX_SOCKET_ERROR;
        }
    }
    dxstore.set_status(status_t::Done);
    nx_sock_close(waker_upper);
    return 0;
}

int si_socket_init() {
    Deckastore& dxstore = Deckastore::get();
    if (!dxstore.IsSingleInstanceEnforced())
        return 0;
    const status_t& dxstatus = dxstore.get_status();
    DxWindow& dxwindow = dxstore.window;
    dxstore.add_task(tasks::SingleInstance);
//#ifdef _WIN32
//    if (getenv("")) {
        // TODO: Multiple users.
        // Message existing socket user and other info
        // TODO: if another user launches, allow them to use a different port
        // in another update and a separate branch.
//    }
//#endif
    // Not using AF_UNIX for now because we don't want to pollute the C: drive on windows
    // and Windows usernames can be too long for the socket path. on linux we can just use /tmp/
    // WHEN: bind, connect, unlink, bind
    socket_t sis = socket(AF_INET, SOCK_STREAM, 0);
    if (sis == NX_INVALID_SOCKET) {
        fprintf(stderr, "Failed to ensure single instance functionality.\n");
        dxstore.set_status(status_t::Done);
        dxstore.remove_task(tasks::SingleInstance);
        return NX_SOCKET_ERROR;
    }

    sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(38674);

    if (bind(sis, (sockaddr*)&addr, sizeof(addr)) == NX_SOCKET_ERROR) {
        fprintf(stderr, "Attempting to summon existing instance.\n");
        dxstore.set_status(status_t::Done);
        nx_sock_close(sis);
        dxstore.remove_task(tasks::SingleInstance);
        return si_summon_instance();
    }

    if (listen(sis, SOMAXCONN) == NX_SOCKET_ERROR) {
        fprintf(stderr, "Failed to listen for wakeup calls.\n");
        dxstore.set_status(status_t::Done);
        nx_sock_close(sis);
        dxstore.remove_task(tasks::SingleInstance);
        return NX_SOCKET_ERROR;
    }
    dxstore.set_status(status_t::Running);

    pollfd pole[1];
    pole[0].fd = sis;
    pole[0].events = POLLIN;
    do {
        int pres = poll(pole, 1, 2000);
        if (!pres) {
            continue;
        } else if (pres == NX_SOCKET_ERROR) {
        }
        pole[0].revents = 0;

        socket_t sndi = accept(sis, nullptr, nullptr);
        if (sndi == NX_INVALID_SOCKET) {
            continue;
        }

        char buf[1] = {0};
        int rres = recv(sndi, buf, 1, 0);
        if (rres == NX_SOCKET_ERROR) {
            // TODO: error handling
        }

        if (dxwindow.get()) {
            if (*buf == *wakeywakeybyte) {
                printf("Received wakeup signal.\n");
                dxwindow.show();
            }
        }

        nx_sock_close(sndi);
    } while (!(dxstatus == status_t::Done || dxstatus == status_t::Exiting) || dxstore.get_tasks() != tasks::SingleInstance);

    nx_sock_close(sis);
    dxstore.remove_task(tasks::SingleInstance);
    return 0;
}
