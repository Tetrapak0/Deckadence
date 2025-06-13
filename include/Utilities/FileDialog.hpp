#pragma once

#include <string>

#include "../../external/NativeFileDIalogs-Extended/include/nfd.hpp"
#include "../../external/NativeFileDIalogs-Extended/include/nfd_glfw3.h"

using std::string;

extern string nfd_open_file();
extern string nfd_open_exe();
extern string nfd_open_dir();
