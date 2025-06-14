#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#else
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif

#include "../../include/Utilities/FileDialog.hpp"

#include "../../include/GUI/GUI.hpp"
#include "../../include/Config/Deckastore.hpp"

string nfd_open_file() {
    NFD_Init();
    const DxWindow& dxwindow = Deckastore::get().get_window();
    nfdwindowhandle_t nfdwh{};
    NFD_GetNativeWindowFromGLFWWindow(dxwindow.get(), &nfdwh);
    nfdfilteritem_t nfdfilter[1] = {{"All Files", "*"}};
    nfdopendialogu8args_t nfdargs{};
    nfdargs.defaultPath = nullptr;
    nfdargs.filterList = nfdfilter;
    nfdargs.filterCount = 1;
    nfdargs.parentWindow = nfdwh;
    nfdchar_t* out = nullptr;
    nfdresult_t res = NFD_OpenDialogU8_With(&out, &nfdargs);
    NFD_Quit();
    if (res != NFD_OKAY)
        return "";
    return out;
}

string nfd_open_exe() {
#ifdef _WIN32
    NFD_Init();
    const DxWindow& dxwindow = Deckastore::get().get_window();
    nfdwindowhandle_t nfdwh{};
    NFD_GetNativeWindowFromGLFWWindow(dxwindow.get(), &nfdwh);
    nfdu8filteritem_t nfdfilter[1]{{"Executables", "exe"}};
    nfdopendialogu8args_t nfdargs{};
    nfdargs.defaultPath = nullptr;
    nfdargs.filterList = nfdfilter;
    nfdargs.filterCount = 1;
    nfdargs.parentWindow = nfdwh;
    nfdchar_t* out = nullptr;
    nfdresult_t res = NFD_OpenDialogU8_With(&out, &nfdargs);
    NFD_Quit();
    if (res != NFD_OKAY)
        return "";
    return out;
#endif
    return nfd_open_file();
}

string nfd_open_dir() {
    NFD_Init();
    const DxWindow& dxwindow = Deckastore::get().get_window();
    nfdwindowhandle_t nfdwh{};
    NFD_GetNativeWindowFromGLFWWindow(dxwindow.get(), &nfdwh);
    nfdpickfolderu8args_t nfdargs{};
    nfdargs.defaultPath = nullptr;
    nfdargs.parentWindow = nfdwh;
    nfdchar_t* out = nullptr;
    nfdresult_t res = NFD_PickFolderU8_With(&out, &nfdargs);
    NFD_Quit();
    if (res != NFD_OKAY)
        return "";
    return out;
}
