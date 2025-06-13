#pragma once

#include <string>

#ifndef _WIN32
#define NFD_PORTAL
#endif

#include "../../external/NativeFileDialogs-Extended/include/nfd.hpp"
#include "../../external/NativeFileDialogs-Extended/include/nfd_glfw3.h"

using std::string;

extern string nfd_open_file();
extern string nfd_open_exe();
extern string nfd_open_dir();
