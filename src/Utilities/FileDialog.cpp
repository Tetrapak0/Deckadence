#include "../../include/Utilities/FileDialog.hpp"

string nfd_open_file() {
    NFD_Init();
    nfdfilteritem_t filter[] = {{"All Files", "*"}};
    nfdchar_t* out = nullptr;
    nfdresult_t res = NFD_OpenDialog(&out, filter, 1, nullptr);
    NFD_Quit();
    if (res != NFD_OKAY)
        return "";
    return out;
}

string nfd_open_exe() {
#ifdef _WIN32
    NFD_Init();
    nfdfilteritem_t filter[] = {{"Executables", "exe"}};
    nfdchar_t* out = nullptr;
    nfdresult_t res = NFD_OpenDialog(&out, filter, 1, nullptr);
    NFD_Quit();
    if (res != NFD_OKAY)
        return "";
    return out;
#endif
    return nfd_open_file();
}

string nfd_open_dir() {
    NFD_Init();
    nfdchar_t* out = nullptr;
    nfdresult_t res = NFD_PickFolder(&out, nullptr);
    NFD_Quit();
    if (res != NFD_OKAY)
        return "";
    return out;
}
