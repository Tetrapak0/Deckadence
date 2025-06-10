cmake_minimum_required(VERSION 3.15)
project(NexusShell)
set(CMAKE_C_STANDARD   17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED   True)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

file(GLOB NxShSource ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/src/config/*.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/*.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/src/net/*.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/src/util/*.cpp)
file(GLOB ImGuiSource ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/*.cpp)

add_executable(NexusShell ${NxShSource} ${ImGuiSource})
# TODO: Add libraries.
if(CMAKE_BUILD_TYPE MATCHES Debug)
	target_compile_definitions(NexusShell PRIVATE _DEBUG)
endif()
