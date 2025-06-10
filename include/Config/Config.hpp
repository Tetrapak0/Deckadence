#pragma once

#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS 1
    #define CFG_VAR "APPDATA"
    #define CFG_REQ "Deckadence"
    #define USR_VAR "USERNAME"
#elif defined(__LINUX__)
    #define CFG_VAR "HOME"
    #define CFG_REQ ".config/Deckadence"
    #define USR_VAR "LOGNAME"
#endif

#include <filesystem>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "../GUI/GUI.hpp"
#include "Deckastore.hpp"

#include "../../external/jsonhpp/json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;
using std::mutex;
using std::string;
using std::unordered_map;
using std::vector;

extern fs::path get_cfg_dir();
extern uint64_t generate_uuid();
extern int      check_config();
extern int      run_oobe();
