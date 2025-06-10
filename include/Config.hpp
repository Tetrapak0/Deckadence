#pragma once

#define CURRENT_SHELL        NxSh::shells[g_shell_uuid]
#define CURRENT_PROFILE      CURRENT_SHELL.profiles[CURRENT_SHELL.profile_idx]
#define CURRENT_PROFILE_SELF shell.profiles[shell.current_profile]

#define CURRENT_ITEM       CURRENT_PROFILE[index]
#define CURRENT_ITEM_LOOP  CURRENT_PROFILE[i]

#ifdef _WIN32
 #define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <mutex>
#include <variant>
#include <memory>

#include "Server.hpp"
#include "GUI/Button.hpp"

#include "../external/jsonhpp/json.hpp"

//#include "../../external/zlib/include/zlib.h"
//#include "../../external/zlib/include/zconf.h"

//#pragma comment(lib, "../external/zlib/bin/zlib.lib")

using std::string;
using std::vector;
using std::variant;
using std::ifstream;
using std::ofstream;
using std::istreambuf_iterator;
using std::shared_ptr;
using std::unique_ptr;
using std::mutex;
using std::unordered_map;

using std::filesystem::exists;
using std::filesystem::remove;
using std::filesystem::create_directory; // TODO: Schedule for removal. on client too.
using std::filesystem::create_directories;

using json = nlohmann::ordered_json;

namespace NxSh {
// TODO: when connecting check hash of config file and all thumbnails before resending.
class Profile {
public:
    string name     = "";
	int    columns	= 6;
	int    rows     = 4;
    int    size     = columns * rows;
	//int    page_idx = 0;

    // repalce with vector<Page>
    vector<GUI::Item>* items = &m_items;
    vector<GUI::Item>* editor_items = &m_items;

private:
    vector<GUI::Item> m_items;

    GUI::Folder* m_root = nullptr;
    GUI::Folder* m_editor_root = nullptr;
public:
    void set_root(GUI::Folder* root);
    void set_editor_root(GUI::Folder* root);
};

class Shell {
private:
    string m_uuid;
public:
    string nickname;

    socket_t client;
    int res = 1, send_res = 1;

    json config;

    int profile_idx = 0;
    vector<Profile> profiles;

    mutex lock;

    string get_uuid() {return m_uuid;}

    Shell() = delete;
    Shell(string uuid) : m_uuid(uuid) {}
    Shell(const Shell& other) : m_uuid(other.m_uuid),
                                nickname(other.nickname),
                                client(other.client),
                                config(other.config),
                                profile_idx(other.profile_idx),
                                profiles(other.profiles) {}
    Shell(const Shell* other) : m_uuid(other->m_uuid),
                                nickname(other->nickname),
                                client(other->client),
                                config(other->config),
                                profile_idx(other->profile_idx),
                                profiles(other->profiles) {}
    Shell(const Shell&& other) : m_uuid(other.m_uuid),
                                nickname(other.nickname),
                                client(other.client),
                                config(other.config),
                                profile_idx(other.profile_idx),
                                profiles(other.profiles) {}
    Shell& operator=(Shell& other) {
        if (this != &other) {
            m_uuid = other.get_uuid();
            nickname = other.nickname;
            client = other.client;
            res = other.res;
            send_res = other.send_res;
            config = other.config;
            profile_idx = other.profile_idx;
            profiles = other.profiles;
        }
        return *this;
    }
};

extern unordered_map<string, Shell> shells;

extern mutex shells_lock;

extern int configure_shell(Shell& shell);

extern void clear_button(int profile, int page, int button);
extern void reconfigure_shell(Shell& shell);
extern void config_write(Shell& shell);
}
