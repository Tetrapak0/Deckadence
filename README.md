# Deckadence
<p align="center">
  <img src="https://github.com/Tetrapak0/NexusShell/blob/main/icon.png?raw=true" alt="Icon"/>
</p>

## Index:
- [Acquisiton](#acquisition)
- [Build](#build)
  - [1. Clone repository](#1-clone-repository)
  - [2. Install dependencies](#2-install-dependencies)
  - [3. Configure CMake](#3-configure-cmake)
  - [4. Build](#4-build)
## Acquisition:
- [Prebuilt binaries](https://github.com/Tetrapak0/NexusShell/releases) ([Install dependencies first](#2-install-dependencies) if you are using Linux)
- [Build it yourself](#build)
## Build:
#### 1. Clone repository
##### 1.1 Install git:
- Arch-based: `sudo pacman -Syyu git`
- Debian-based: `sudo apt update && sudo apt install git`
- Fedora-based: `sudo dnf upgrade --refresh && sudo dnf install git`
- MSYS: `pacman -Syyu git`
##### 1.2 Clone
> Note: if you are on Windows, do this step in the MSYS shell

`git clone https://github.com/Tetrapak0/Deckadence --depth 1`
#### 2. Install dependencies
#### 2.1 Linux
>During development for Linux, Deckadence is only built using clang++.
- Arch-based: `sudo pacman -Syyu freetype2 cmake base-devel clang glfw dbus mesa libx11 libxrandr wayland`
- Debian-based: `sudo apt update && sudo apt install libfreetype-dev cmake build-essential clang libglfw3 libdbus-1-dev libgl1-dev libglu1-dev libx11-dev libxrandr-dev libwayland-dev`
- Fedora-based: `sudo dnf upgrade --refresh && sudo dnf install freetype-dev cmake clang glfw dbus dbus-devel mesa-libGL-devel mesa-libGLU-devel libx11-devel libxrandr-devel wayland-devel && sudo dnf groupinstall "Development Tools" "Development Libraries"`
#### 2.2 Windows
> ⚠ MSVC is not supported because Microsoft C++ is completely and utterly repugnant and should be EOL'd. Even their devs hate using it. It's true. ⚠
>
> During development for Windows, Deckadence is only built using clang++ via MSYS for the aforementioned reasons.
1. Install MSYS2 from https://www.msys2.org/
2. Open the Clang64 environment and run the following commands to install dependencies:
    > Note that you may be asked to quit the session to update packages. In that case, please re-run the last command.
    ```shell
    pacman -Syyu mingw-w64-clang-x86_64-toolchain mingw-w64-clang-x86_64-glfw mingw-w64-clang-x86_64-cmake mingw-w64-clang-x86_64-freetype
    ```
3. Enter your MSYS home directory: `cd ~`
4. Install git via `pacman -S git`
5. [Clone repository](#1-clone-repository)
#### 3. Configure CMake
```shell
cd Deckadence
cmake -S . -B build -DCMAKE_BUILD_TYPE=MinSizeRel
```
#### 4. Build
```shell
cd build
cmake --build . --config MinSizeRel
```
