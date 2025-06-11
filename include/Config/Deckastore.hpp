#pragma once

#define DX_CONFIG_BYTE 0x43
#define DX_DISCONNECT_BYTE 0x44
#define DX_EXECUTE_BYTE 69
#define DX_WAKEUP_BYTE 0x57

#include "../external/Tetrapak0/NexusSockets.h"

#include "../Client/Client.hpp"
#include "../GUI/GUI.hpp"
#include "../GUI/DxWindow.hpp"
#include "../Server/DiscoveryService.hpp"

#include <unordered_map>

using std::unordered_map;

enum class status_t {
    RUNNING     = 0,
    DONE        = 1,
    EXITING     = 2,
    RESTART     = 4,
    RESTARTING  = 8,
};
namespace Deckadence {
enum class mode_t {
    SERVER      = 0,
    CLIENT      = 1
};
}
enum class tasks : uint64_t {
    NONE        = 0,
    SIS         = 1,
    SERVER      = 2,
    CLIENT      = 2,
    DISCOVERY   = 4,
};
inline tasks operator|(tasks lhs, tasks rhs) {
    return static_cast<tasks>(static_cast<uint64_t>(lhs) | static_cast<uint64_t>(rhs));
}
inline tasks& operator|=(tasks& lhs, const tasks rhs) {
    lhs = lhs | rhs;
    return lhs;
}
inline tasks operator&(tasks lhs, tasks rhs) {
    return static_cast<tasks>(static_cast<uint64_t>(lhs) & static_cast<uint64_t>(rhs));
}
inline tasks& operator&=(tasks& lhs, const tasks rhs) {
    lhs = lhs & rhs;
    return lhs;
}
inline tasks operator~(tasks rhs) {
    return static_cast<tasks>(~static_cast<uint64_t>(rhs));
}

class Deckastore {
    friend int main(int argc, char** argv);
    friend int check_config();
    friend int draw_pages();

    tasks running_tasks = tasks::NONE;
    status_t status = status_t::RUNNING;

    Deckadence::mode_t mode = Deckadence::mode_t::SERVER;
    bool discoverable = true;
    bool singleinstance = true;
    uint16_t port = 32018;

    uint64_t selected_client_id = 0; // NOTE: 0 mustn't be a valid UUID
    unordered_map<uint64_t, Client> clients;
    vector<NetworkInterface> interfaces;

    mutex lock;

    json config;

    DxWindow window;

    static void reset();

    void erase_client(uint64_t uuid);

    void set_mode(Deckadence::mode_t mode);

    void set_port(uint16_t port);

    bool& get_discoverable_ref();

    void disable_sis();
public:
    int  draw_item_properties = -1;
    bool draw_properties      = false;
    bool draw_settings        = false;

    [[nodiscard]] bool has_sis() const;

    int create_window(const char* title, const Vec2<int>& size = {800, 600}, int fullscreen = 0,
                      const std::vector<std::pair<int, int>>& hints = {},
                      const Vec2<int>& min_size = {800, 600}, const Vec2<int>& max_size = {GLFW_DONT_CARE, GLFW_DONT_CARE});
    [[nodiscard]] const DxWindow& get_window() const;
    void destroy_window();

    static Deckastore& get();

    void net_query_interfaces();
    vector<NetworkInterface>& get_ifaces_ref();

    void set_discoverable(bool d);
    [[nodiscard]] const bool& get_discoverable() const;

    Client& retrieve_client(uint64_t uuid);
    Client& retrieve_current_client();

    json& get_config();

    [[nodiscard]] const uint64_t& get_selected_uuid() const;
    void set_selected_uuid(uint64_t uuid);

    const decltype(clients)& get_client_map();
    bool client_exists(uint64_t client);
    void insert_client(Client& client);
    void insert_client(uint64_t uuid, socket_t socket);
    void disconnect_client(uint64_t uuid, const string& reason);

    void set_status(status_t status);
    [[nodiscard]] const status_t& get_status() const;

    [[nodiscard]] Deckadence::mode_t get_mode() const;

    [[nodiscard]] uint16_t get_port() const;

    [[nodiscard]] const tasks& get_tasks() const;
    void add_task(tasks task);
    void remove_task(tasks task);

    ~Deckastore() = default;
private:
    Deckastore() = default;
};
