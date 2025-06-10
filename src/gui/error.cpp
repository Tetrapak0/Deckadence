#include "../../include/GUI.hpp"

void error_dialog(int type, string message) {
    if (!type) message += SDL_GetError();
    const SDL_MessageBoxButtonData button_data[] = {
        {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "OK"}
    };
    int buttonid;
    const SDL_MessageBoxData msgbox_data = {
        SDL_MESSAGEBOX_ERROR, NULL, "Error!", message.c_str(),
        SDL_arraysize(button_data), button_data, NULL
    };
    SDL_ShowMessageBox(&msgbox_data, &buttonid);
}
