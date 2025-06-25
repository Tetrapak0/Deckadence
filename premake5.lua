workspace "Deckadence"
    configurations {"Debug", "Release"}
    if (_ACTION or ""):find("vs") then
        platforms {"Win64"}
    end

project "Deckadence"
    kind "WindowedApp"
    language "C++"

    files {"src/**.cpp", "include/**.hpp", "external/NativeFileDialogs-Extended/nfd_win.cpp", "external/ImGui/*", "external/Tetrapak0/*", "external/stb/*"}
    filter {"system:windows"}
        flags {"MultiProcessorCompile", "NoImportLib"}
        files {"Deckadence.rc", "**.ico"}
        vpaths {["*"] = {"*.rc", "**.ico"}}
    filter {}
    if (_ACTION or ""):find("vs") then
        cppdialect "c++17"
        cdialect "c17"
        links {"opengl32", "ws2_32", "iphlpapi", "dwmapi"}
        libdirs {"external/**/lib"}
        links {"freetype", "glfw3"}
    else
        cppdialect "gnu++17"
        cdialect "gnu17"
        links {"glfw", "GL", "freetype"}
        libdirs {os.findlib("dbus-1")}
        links {"dbus-1"}
    end

    filter "configurations:Debug"
        defines {"_DEBUG"}
        symbols "On"

    filter "configurations:Release"
        optimize "On"
