#include "pch.h"

#define IMGUI_DEFINE_MATH_OPERATORS // https://github.com/ocornut/imgui/issues/2832

# include "imgui-docking/imgui.h"
# include "theme.h"

void setTheme(THEME theme)
{
    ImGui::StyleColorsDark();

    // spacing and padding is not overriden by changing colors

    auto io = ImGui::GetIO();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        //MyApp::SetupImGuiStyle();

        //style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        style.WindowBorderSize = 0.0f;
        style.PopupBorderSize = 0.0f;

        // for fully transparent windows backgrounds
        //style.Colors[ImGuiCol_WindowBg].w = 0.4f;

        // for semi transparent globally
        // TODO. hovering window = .9?
        // style.Alpha = .6f;

        style.WindowRounding = 4.0f;
        style.PopupRounding = 4.0f;
        style.ChildRounding = 0.0f;

        // demo style editor :D
        style.WindowPadding = { 0.0f, 0.0f };
        style.WindowTitleAlign = { 0.5f, 0.5f };
        style.ItemSpacing = { 8.0f, 8.0f };
        style.CellPadding = { 8.0f, 4.0f };

        style.FrameRounding = 2.0f;
        //style.FramePadding = { 4.0f, 4.0f };
        style.FramePadding = { 10.0f, 8.0f };

        style.SelectableRounding = 2.0f;

        style.IndentSpacing = 20;

        style.GrabMinSize = 8;
        style.GrabRounding = 2;

        //style.SeparatorTextPadding = 3;

        style.ItemSpacing = { 10, 10 };

        style.DisabledAlpha = 0.4f;

        //style.Colors[ImGuiCol_Button] = { 0, 0, 0, 0.8f };
        //style.Colors[ImGuiCol_ButtonHovered] = { 0, 0, 0, 0.7f };
        //style.Colors[ImGuiCol_ButtonActive] = { 0, 0, 0, 0.6f };
        
        style.Colors[ImGuiCol_Text] = { 0.910f, 0.929f, 0.949f, 1.0f };
        style.Colors[ImGuiCol_TextDisabled] = { 0.420f, 0.478f, 0.553f, 1.0f };
        style.Colors[ImGuiCol_WindowBg] = { 0.031f, 0.047f, 0.063f, 1.0f };
        style.Colors[ImGuiCol_ChildBg] = { 0.031f, 0.047f, 0.063f, 1.0f };
        style.Colors[ImGuiCol_PopupBg] = { 0.051f, 0.067f, 0.090f, 0.98f };
        style.Colors[ImGuiCol_Border] = { 0.110f, 0.137f, 0.200f, 1.0f };
        style.Colors[ImGuiCol_FrameBg] = { 0.063f, 0.086f, 0.125f, 1.0f };
        style.Colors[ImGuiCol_FrameBgHovered] = { 0.078f, 0.108f, 0.157f, 1.0f };
        style.Colors[ImGuiCol_FrameBgActive] = { 0.118f, 0.227f, 0.373f, 1.0f };
        style.Colors[ImGuiCol_TitleBg] = { 0.086f, 0.106f, 0.133f, 1.0f };
        style.Colors[ImGuiCol_TitleBgActive] = { 0.086f, 0.106f, 0.133f, 1.0f };
        style.Colors[ImGuiCol_TitleBgCollapsed] = { 0.086f, 0.106f, 0.133f, 1.0f };
        style.Colors[ImGuiCol_MenuBarBg] = { 0.051f, 0.067f, 0.090f, 1.0f };
        style.Colors[ImGuiCol_ScrollbarBg] = { 0.031f, 0.047f, 0.063f, 1.0f };
        style.Colors[ImGuiCol_ScrollbarGrab] = { 0.110f, 0.137f, 0.200f, 1.0f };
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = { 0.000f, 0.831f, 1.000f, 0.65f };
        style.Colors[ImGuiCol_ScrollbarGrabActive] = { 0.000f, 0.831f, 1.000f, 0.9f };
        style.Colors[ImGuiCol_CheckMark] = { 0.000f, 1.000f, 0.529f, 1.0f };
        style.Colors[ImGuiCol_Button] = { 0.063f, 0.086f, 0.125f, 1.0f };
        style.Colors[ImGuiCol_ButtonHovered] = { 0.078f, 0.108f, 0.157f, 1.0f };
        style.Colors[ImGuiCol_ButtonActive] = { 0.118f, 0.227f, 0.373f, 1.0f };
        style.Colors[ImGuiCol_Header] = { 0.063f, 0.086f, 0.125f, 1.0f };
        style.Colors[ImGuiCol_HeaderHovered] = { 0.000f, 0.831f, 1.000f, 0.18f };
        style.Colors[ImGuiCol_HeaderActive] = { 0.000f, 0.831f, 1.000f, 0.28f };
        style.Colors[ImGuiCol_Separator] = { 0.110f, 0.137f, 0.200f, 1.0f };
        style.Colors[ImGuiCol_ModalWindowDimBg] = { 0.0f, 0.0f, 0.0f, 0.82f };

    }

}

void ToggleButton(const char* str_id, bool* v)
{
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.55f;
    float radius = height * 0.50f;

    ImGui::InvisibleButton(str_id, ImVec2(width, height));
    if (ImGui::IsItemClicked())
        *v = !*v;

    float t = *v ? 1.0f : 0.0f;

    ImGuiContext& g = *GImGui;
    float ANIM_SPEED = 0.08f;
    if (g.LastActiveId == g.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
    {
        float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
        t = *v ? (t_anim) : (1.0f - t_anim);
    }

    static const auto color_toggle_background = ImColor::HSV(fmod(-0.02f, 1.0f) / 14.0f, 0.4f, 1.0f, 1.0f);


    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), color_toggle_background, height * 0.5f);
    draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
}
