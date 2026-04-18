#pragma once

#include "../../external/Tetrapak0/NexusSockets.h"

#include "ItemTypeRegistry.hpp"
#include "Client/Client.hpp"
#include "GUI/GUI.hpp"
#include "GUI/DxWindow.hpp"
#include "Server/DiscoveryService.hpp"

#include <unordered_map>

using std::unordered_map;

enum class status_t {
    Running     = 0,
    Done        = 1,
    Exiting     = 2,
    Restart     = 4,
    Restarting  = 8,
};
namespace Deckadence {
enum class mode_t {
    Server      = 0,
    Client      = 1,
    Undefined   = 0xFFFF
};
}
enum class tasks : uint64_t {
    None            = 0,
    SingleInstance  = 1,
    Server          = 2,
    Client          = 2,
    Discovery       = 4,
    Setup           = 8
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
    friend int  main(int argc, char** argv);
    friend int  check_config();
    friend void draw_pages();

    tasks running_tasks = tasks::None;
    status_t status = status_t::Running;

    Deckadence::mode_t mode = Deckadence::mode_t::Server;
    bool discoverable = true;
    bool singleinstance = true;
    uint16_t port = 32018;

    uint64_t selected_client_id = DX_INVALID_UUID;
    unordered_map<uint64_t, Client> clients;
    unordered_map<string, unique_ptr<Texture>> textureStore;
    vector<NetworkInterface> interfaces;

    json config;

    static void reset();

    void erase_client(uint64_t uuid);

    void set_mode(Deckadence::mode_t mode);

    void set_port(uint16_t port);

    bool& get_discoverable_ref();

    void disable_sis();
public:
    DxWindow window;
    mutex lock;
    ItemTypeRegistry& type_registry = ItemTypeRegistry::get();
    ImFont* header  = nullptr;
    ImFont* regular = nullptr;

    int  draw_item_properties = -1;
    bool draw_properties      = false;
    bool draw_settings        = false;

    [[nodiscard]] bool IsSingleInstanceEnforced() const;

    int create_window(const char* title, const Vec2<int>& size = {800, 600}, int fullscreen = 0,
                      const std::vector<std::pair<int, int>>& hints = {},
                      const Vec2<int>& min_size = {800, 600}, const Vec2<int>& max_size = {GLFW_DONT_CARE, GLFW_DONT_CARE});
    void destroy_window();

    static Deckastore& get();

    void net_query_interfaces();
    vector<NetworkInterface>& get_ifaces_ref();

    void set_discoverable(bool d);
    [[nodiscard]] const bool& get_discoverable() const;

    Client& retrieve_client(uint64_t uuid);
    Client& retrieve_current_client();

    Texture* create_texture(string name, fs::path path);
    Texture* create_texture(string name, const uint8_t* data, int size);

    json& get_config();

    [[nodiscard]] const uint64_t& get_selected_uuid() const;
    void set_selected_uuid(uint64_t uuid);

    const decltype(clients)& get_client_map();
    bool client_exists(uint64_t client);
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
