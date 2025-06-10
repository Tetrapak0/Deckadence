#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
// gcc/clang -municode

#define NXSH_VERSION "Alpha 1.5" // URGENT: Update each version

#ifdef _WIN32
 #define _CRT_SECURE_NO_WARNINGS 1
 #define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#define WIN32_LEAN_AND_MEAN

// x is the desired standard output
// TODO:  call custom function
//        check if __PROD is defined instead
//        Always run under Release unless debugging; include symbols
//        Prod is the configuration for release.
#ifdef _DEBUG
 #define LOG(x, ...) fprintf(x, __VA_ARGS__)
#else
 #define LOG(x, ...)
#endif

#include <windows.h>
#include <winuser.h>
#include <shellapi.h>

#pragma comment(lib, "user32.lib")
#endif

#include <iostream>
#include <thread>
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>

using std::cin;
using std::cerr;

using std::string;
using std::vector;
using std::thread;
using std::unordered_map;
using std::wstring;

using std::to_string;
using std::filesystem::create_directories;

extern bool done;
