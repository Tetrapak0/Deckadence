#include "Config/Deckastore.hpp"
#include "Config/Config.hpp"
#include "GUI/DxWindow.hpp"
#include "Server/DiscoveryService.hpp"
#include "Server/Message.hpp"

void Deckastore::reset() {
    Deckastore& self = get();
    self.status = status_t::Running;
    self.mode = Deckadence::mode_t::Server;
    self.discoverable = true;
    self.port = 32018;
    self.draw_properties = false;
    self.draw_item_properties = -1;
    self.draw_settings = false;
    self.selected_client_id = 0;
    self.clients.clear();
    self.interfaces.clear();
    self.type_registry.reset();
    self.window.destroy();
    self.config.clear();
}

void Deckastore::erase_client(const uint64_t uuid) {
    clients.erase(uuid);
}
void Deckastore::set_mode(const Deckadence::mode_t mode) {
    lock.lock();
    this->mode = mode;
    lock.unlock();
}
void Deckastore::set_port(uint16_t port) {
    lock.lock();
    this->port = port;
    lock.unlock();
}
bool& Deckastore::get_discoverable_ref() {
    return discoverable;
}
void Deckastore::disable_sis() {
    singleinstance = false;
}

bool Deckastore::IsSingleInstanceEnforced() const {
    return singleinstance;
}

int Deckastore::create_window(const char* title, const Vec2<int>& size, const int fullscreen,
                              const std::vector<std::pair<int, int>>& hints,
                              const Vec2<int>& min_size, const Vec2<int>& max_size) {
    assert(!window.get());
    window.create(title, size, fullscreen, hints, min_size, max_size);
    return window.get() ? 0 : -1;
}
void Deckastore::destroy_window() {
    window.destroy();
}

Deckastore& Deckastore::get() {
    static Deckastore self;
    return self;
}

void Deckastore::net_query_interfaces() {
    vector<NetworkInterface> interfaces = query_interfaces();
    // TODO: Implement hybrid copy; sockets are copied in case of change
    this->interfaces = interfaces;
}
vector<NetworkInterface>& Deckastore::get_ifaces_ref() {
    return this->interfaces;
}

void Deckastore::set_discoverable(const bool d) {
    lock.lock();
    discoverable = d;
    lock.unlock();
}
const bool& Deckastore::get_discoverable() const {
    return discoverable;
}

Client& Deckastore::retrieve_client(const uint64_t uuid) {
    return clients.at(uuid);
}
Client& Deckastore::retrieve_current_client() {
    return retrieve_client(selected_client_id);
}

Texture* Deckastore::create_texture(string name, fs::path path) {
    auto [it, inserted] = this->textureStore.try_emplace(name, std::make_unique<Texture>(path));
    
    if (inserted)
        return it->second.get();
}

Texture* Deckastore::create_texture(string name, const uint8_t* data, int size) {
    auto [it, inserted] = this->textureStore.try_emplace(name, std::make_unique<Texture>(data, size));

    if (inserted)
        return it->second.get();
}

json& Deckastore::get_config() {
    return this->config;
}

const uint64_t& Deckastore::get_selected_uuid() const {
    return selected_client_id;
}
void Deckastore::set_selected_uuid(const uint64_t uuid) {
    lock.lock();
    selected_client_id = uuid;
    lock.unlock();
}
const decltype(Deckastore::clients)& Deckastore::get_client_map() {
    return clients;
}
bool Deckastore::client_exists(const uint64_t client) {
    return clients.find(client) != clients.end();
}
void Deckastore::insert_client(uint64_t uuid, socket_t socket) {
    lock.lock();
    // clients.emplace(std::piecewise_construct,
    //                 std::forward_as_tuple(uuid),
    //                 std::forward_as_tuple(uuid, socket));
    clients.try_emplace(uuid, uuid, socket);
    if (selected_client_id == DX_INVALID_UUID)
        selected_client_id = uuid;
    lock.unlock();
}
// TODO: Move to destructor of Client. can't really right now
void Deckastore::disconnect_client(const uint64_t uuid, const string& reason) {
    lock.lock();
    if (clients.find(uuid) == clients.end()) {
        lock.unlock();
        return;
    }
    Client& c = retrieve_client(uuid);
    if (selected_client_id == c.get_uuid()) {
        // TODO: Add offline config editor; when client not connected
        draw_item_properties = -1;
        draw_properties = false;
        selected_client_id = DX_INVALID_UUID;
    }
    // this is allowed to fail, and it always will if the client disconnects itself
    send(c.socket, string(string(1, (char)MessageType::Disconnect) + reason).c_str(), reason.length()+1, 0);
    if (c.socket != NX_INVALID_SOCKET) {
        nx_sock_close(c.socket);
        c.socket = NX_INVALID_SOCKET;
    }
    if (c.get_nickname().empty())
        printf("%llu disconnected.\n", c.get_uuid());
    else
       printf("%s disconnected.\n", c.get_nickname().c_str());
    erase_client(uuid);
    if (clients.size())
        selected_client_id = clients.begin()->first;
    lock.unlock();
}

void Deckastore::set_status(const status_t status) {
    lock.lock();
    this->status = status;
    lock.unlock();
}
const status_t& Deckastore::get_status() const {
    return status;
}

Deckadence::mode_t Deckastore::get_mode() const {
    return mode;
}

uint16_t Deckastore::get_port() const {
    return port;
}

const tasks& Deckastore::get_tasks() const {
    return running_tasks;
}
void Deckastore::add_task(const tasks task) {
    running_tasks |= task;
}
void Deckastore::remove_task(const tasks task) {
    running_tasks &= ~task;
}
