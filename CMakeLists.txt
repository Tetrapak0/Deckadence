cmake_minimum_required(VERSION 3.15)
project(Deckadence)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)
include_directories(include/Client)
include_directories(include/Config)
include_directories(include/GUI)
include_directories(include/Server)
include_directories(include/Utilities)

file(GLOB DxSrc ${CMAKE_CURRENT_LIST_DIR}/src/Client/*.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/Config/*.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/GUI/*.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/Server/*.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/Utilities/*.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB ImSrc ${CMAKE_CURRENT_LIST_DIR}/external/imgui/*.cpp)

if (WIN32)
    file(GLOB NFDExSrc ${CMAKE_CURRENT_LIST_DIR}/external/NativeFileDialogs-Extended/nfd_win.cpp)
elseif (UNIX AND NOT APPLE)
    file(GLOB NFDExSrc ${CMAKE_CURRENT_LIST_DIR}/external/NativeFileDialogs-Extended/nfd_portal.cpp)
endif()

add_executable(Deckadence ${DxSrc} ${ImSrc} ${NFDExSrc})
set_property(TARGET Deckadence PROPERTY CXX_STANDARD 17)

# must be ran with -DCMAKE_BUILD_TYPE=Debug
if (${CMAKE_BUILD_TYPE} MATCHES Debug)
    target_compile_definitions(Deckadence PRIVATE _DEBUG)
endif()

if (WIN32)
    target_link_libraries(Deckadence glfw3 opengl32 freetype dwmapi ws2_32 iphlpapi)
elseif (UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(DBUS REQUIRED dbus-1)
    target_include_directories(Deckadence PRIVATE ${DBUS_INCLUDE_DIRS})
    target_link_libraries(Deckadence glfw GL freetype ${DBUS_LINK_LIBRARIES})
endif()
