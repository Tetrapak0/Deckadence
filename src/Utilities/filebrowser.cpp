#include "../../include/GUI/GUI.hpp"
#include "../../include/Header.hpp"

namespace NxSh {

string open_file(int type) {
    NFD_Init();
    nfdchar_t* outPath = nullptr;
    nfdresult_t result;
    switch (type) {
        case 0: { // File to execute; TODO: add multi file
            nfdfilteritem_t filterItem[] = { {"All Files", "*" } };
            result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);
            break;
        } case 1: { // icon picker
            nfdfilteritem_t filterItem[] = { {"Image files", "png,jpg,jpeg,ico,bmp" } };
            result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);
            break;
        } case 2: { // directory launcher; TODO: same as case 0
            result = NFD_PickFolder(&outPath, NULL);
            break;
        }
    }
    NFD_Quit();
    if (result == NFD_OKAY) {
        LOG(stdout, "File browser returned following path: %s\n", outPath);
        return outPath;
    }
    LOG(stdout, "User cancelled browser dialog.\n");
    return "";
}

}