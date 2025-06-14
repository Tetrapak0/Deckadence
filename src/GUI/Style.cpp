#include "../../include/GUI/GUI.hpp"

// TODO: Add custom style
// TODO: Change selectable color
void gui_set_colors() {
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text]                 = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]         = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_ChildBg]              = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PopupBg]              = ImVec4(0.03f, 0.03f, 0.03f, 1.0f);
    colors[ImGuiCol_FrameBg]              = ImVec4(0.10f, 0.10f, 0.10f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.20f, 0.20f, 0.20f, 0.54f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TitleBg]              = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_CheckMark]            = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_SliderGrab]           = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_Button]               = ImVec4(0.10f, 0.10f, 0.10f, 0.54f);
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_Separator]            = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_SeparatorHovered]     = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_SeparatorActive]      = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_ResizeGrip]           = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_ResizeGripHovered]    = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_ResizeGripActive]     = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_Tab]                  = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabHovered]           = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabActive]            = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
    colors[ImGuiCol_TabUnfocused]         = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabUnfocusedActive]   = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]       = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_DragDropTarget]       = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
    colors[ImGuiCol_FrameBg]              = ImVec4(0.12f, 0.12f, 0.12f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.25f, 0.25f, 0.25f, 0.54f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.35f, 0.35f, 0.35f, 0.54f);
    colors[ImGuiCol_CheckMark]            = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
}

void gui_set_style() {
    ImGuiStyle& style      = ImGui::GetStyle();
    style.ItemSpacing      = ImVec2(0.0f, 0.0f);
    style.ItemInnerSpacing = ImVec2(0.0f, 0.0f);
    style.GrabMinSize      = 10;
    style.WindowBorderSize = 0;
    style.ChildBorderSize  = 0;
    style.PopupBorderSize  = 0;
    style.FrameBorderSize  = 0;
    style.TabBorderSize    = 0;
    style.FrameRounding    = 0.0f;
    style.WindowRounding   = 0.0f;
}
