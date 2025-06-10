#include "../include/Header.hpp"
#include "../include/GUI/GUI.hpp"
#include "../include/Server.hpp"

#include "../external/jsonhpp/json.hpp"
#include <processenv.h>
using json = nlohmann::json;
#include <stdio.h>
#include <fstream>

namespace NxSh {

extern std::filesystem::path get_cfg_path();

typedef struct nxsh_flags_s {
    int gflags = GUI::GF_NONE;
} nxsh_flags;

static nxsh_flags parse_args(const int argc, const char*** const argv) {
    nxsh_flags out;
    for (int i = 0; i < argc; ++i) {
        if (!strcmp((*argv)[i], "-n") || !strcmp((*argv)[i], "--nowindow"))
            out.gflags |= GUI::GF_HIDDEN;
    }
    return out;
}

std::filesystem::path nxsh_path;

int check_mode() {
    int mode = 0;
    if (std::filesystem::exists(nxsh_path / "config.json")) {
        std::ifstream reader(nxsh_path / "config.json");
        json config = json::parse(reader);
        if (config.contains("mode")) {
            if (config["mode"].is_number_integer() && config["mode"].get<int>() >= 0) {
                mode = config["mode"].get<int>();
            } else {
                // TODO:
            }
        } else {

        };
    }
    return mode;
}
}
// URGENT: Switch from connections to ids.size
#ifdef _WIN32
int _main(int argc, char** argv) {
#else
int main(int argc, char** argv) {
#endif
    NxSh::nxsh_path = NxSh::get_cfg_path();
    create_directories(NxSh::nxsh_path);
    NxSh::nxsh_flags flags = NxSh::parse_args(argc, (const char*** const)&argv);
    thread tray_handler(NxSh::GUI::systray_init, 0);
    thread server_handler(server_init);
    NxSh::GUI::init(flags.gflags);

    return 0; // TODO: Use return bit flags from each function to determine success
}

#ifdef _WIN32
    #define main _main
    //#define _DEBUG
    #ifdef _DEBUG
        #define _CRTDBG_MAP_ALLOC
        #include <crtdbg.h>
        #include <consoleapi.h>
    #endif
INT APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     PSTR pCmdLine, INT nCmdShow) {
    #ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    AllocConsole();
    AttachConsole(GetCurrentProcessId());

    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);
    freopen("CON", "r", stdin);

    printf("Console initialized successfully!\n"
           "NexusShell ver: %s. _DEBUG is defined.\n", NXSH_VERSION);
    char pwd[MAX_PATH] = {};
    GetCurrentDirectoryA(MAX_PATH, pwd);
    printf("%s\n", pwd);
    #endif
    int ret = main(__argc, __argv);
    #ifdef _DEBUG
    FreeConsole();
    #endif
    return ret;
}
#endif
