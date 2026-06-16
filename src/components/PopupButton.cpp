#include "pch.h"

#include "PopupButton.h"

PopupButton::PopupButton(std::string title, std::string icon, std::vector<Action> const actions, int i, std::string tooltip):
	_title(title),
	_icon(icon),
    _actions(actions),
    _i(i),

	//popup_name(title + "_popup")
	_popup_name(title),
    _tooltip(tooltip)
{
	
}

void PopupButton::render2(const ImVec2& size) {

    static auto& style = ImGui::GetStyle();
    ImDrawList* list = ImGui::GetWindowDrawList();

    /*const auto color_button = ImColor::HSV(0, 0.4f, 1, 1.f);
    const auto color_button_hover = ImColor::HSV(0, 0.25f, 1, 1.f);*/

    //static const ImU32 white = ImGui::ColorConvertFloat4ToU32({ 1, 1, 1, 1 });
    //static const ImU32 color_text = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]);

    const ImU32 color_button = (ImU32) ImColor::HSV(1.0f - ((this->_i + 1) / 32.0f), 0.4f, 1.0f, 1.0f);
    const ImU32 color_button_hover = (ImU32) ImColor::HSV(1.0f - ((this->_i + 1) / 32.0f), 0.3f, 1.0f, 1.0f);
    const ImU32 color_secondary_faded = (ImU32) ImColor::HSV(1.0f - ((this->_i + 1) / 32.0f), 0.2f, 1.0f, 0.4f * 1.0f);

    {
        static const auto font = font_text;
        const auto font_size = font->CalcTextSizeA(24, FLT_MAX, 0.0f, this->_title.c_str());

        ImGui::Dummy(size);
        auto hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);

        /* background */
        if (hovered)
            list->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), color_secondary_faded, 5.0f);

        const auto pos = ImGui::GetItemRectMin() + ImVec2(((ImGui::GetItemRectSize().x - font_size.x) / 2) + 10 + 4, 7);
        list->AddText(font, 24, pos, color_button, this->_title.c_str());


        const ImVec2 frame2 = { 20.0f, 20.0f };
        const ImVec2 pos2 = { pos.x - frame2.x - 8.0f, pos.y + 4.0f };


        list->AddImage((void*)_get_texture(this->_icon), pos2, pos2 + frame2, ImVec2(0, 0), ImVec2(1, 1), color_button);

    }


    if (ImGui::IsItemClicked()) {
        if (this->_actions.size() != 1) {
            ImGui::OpenPopup(this->_popup_name.c_str());
        }
        else {
            this->_actions[0].action();
        }
    }

    this->renderPopup();
}

const std::string& PopupButton::getTooltip() const { return this->_tooltip; }

void PopupButton::render() {

	ImDrawList* list = ImGui::GetWindowDrawList();

    const ImU32 color_border = ImGui::ColorConvertFloat4ToU32({ 0.110f, 0.137f, 0.200f, 1.0f });
    const ImU32 color_bg = ImGui::ColorConvertFloat4ToU32({ 0.039f, 0.055f, 0.078f, 1.0f });
    const ImU32 color_bg_hover = ImGui::ColorConvertFloat4ToU32({ 0.063f, 0.086f, 0.125f, 1.0f });
    const ImU32 color_ice = ImGui::ColorConvertFloat4ToU32({ 0.000f, 0.831f, 1.000f, 1.0f });
    const ImU32 color_hot = ImGui::ColorConvertFloat4ToU32({ 1.000f, 0.176f, 0.420f, 1.0f });
    const ImU32 color_muted = ImGui::ColorConvertFloat4ToU32({ 0.420f, 0.478f, 0.553f, 1.0f });

    const ImVec2 frame = { 34.0f, 34.0f };
	ImGui::InvisibleButton(("##" + this->_title).c_str(), frame);

    const bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
    const ImU32 accent = this->_i == 1 ? color_ice : (this->_i == 3 ? color_hot : color_muted);
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();

    list->AddRectFilled(min, max, hovered ? color_bg_hover : color_bg, 2.0f);
    list->AddRect(min, max, hovered ? accent : color_border, 2.0f, 0, 1.0f);

    const ImVec2 icon_pad = { 8.0f, 8.0f };
	list->AddImage((void*)_get_texture(this->_icon), min + icon_pad, max - icon_pad, ImVec2(0, 0), ImVec2(1, 1), hovered ? accent : color_muted);

	if (ImGui::IsItemClicked()) {
        if (this->_actions.size() == 1) {
            this->_actions[0].action();
        }
        else {
		    ImGui::OpenPopup(this->_popup_name.c_str());
        }
	}

    this->renderPopup();
}


void PopupButton::renderPopup() {

    // Always center this window when appearing
    const ImGuiViewport* parent_viewport = ImGui::GetWindowViewport();
    ImVec2 center = parent_viewport->GetCenter();

    ImGuiWindowClass popup_class;
    popup_class.ParentViewportId = parent_viewport->ID;
    popup_class.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoTaskBarIcon;
    ImGui::SetNextWindowClass(&popup_class);
    ImGui::SetNextWindowViewport(parent_viewport->ID);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(450.0f, 0.0f), ImGuiCond_Appearing);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 14.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 5.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.051f, 0.067f, 0.090f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.110f, 0.137f, 0.200f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.910f, 0.929f, 0.949f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.420f, 0.478f, 0.553f, 1.0f));
    if (ImGui::BeginPopupModal(this->_popup_name.c_str(), NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImDrawList* list = ImGui::GetWindowDrawList();

        // handle close
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsWindowHovered() && !ImGui::IsWindowAppearing())
            ImGui::CloseCurrentPopup();

        ImGui::PushFont(font_subtitle);
        {
            int i = 1;

            for (auto& item : this->_actions) {
                const bool checked = item.state ? item.state() : false;
                std::string detail;
                if (item.state)
                {
                    detail = item.description.empty() ? (checked ? "Ativado" : "Desativado") : item.description;
                }
                else
                {
                    detail = item.description;
                }

                const float row_height = 48.0f;
                const ImVec2 row_size = ImVec2(ImGui::GetContentRegionAvail().x, row_height);

                ImGui::InvisibleButton(("##popup_action_" + std::to_string(i)).c_str(), row_size);

                const bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
                const bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left) && !item.disabled;
                const ImVec2 min = ImGui::GetItemRectMin();
                const ImVec2 max = ImGui::GetItemRectMax();
                const ImU32 bg = ImGui::ColorConvertFloat4ToU32({ 0.063f, 0.086f, 0.125f, item.disabled ? 0.45f : 0.78f });
                const ImU32 bg_hover = ImGui::ColorConvertFloat4ToU32({ 0.078f, 0.108f, 0.157f, 1.0f });
                const ImU32 border = ImGui::ColorConvertFloat4ToU32({ 0.110f, 0.137f, 0.200f, 1.0f });
                const ImU32 text = ImGui::ColorConvertFloat4ToU32(item.disabled
                    ? ImVec4{ 0.420f, 0.478f, 0.553f, 1.0f }
                    : ImVec4{ 0.910f, 0.929f, 0.949f, 1.0f });
                const ImU32 muted = ImGui::ColorConvertFloat4ToU32({ 0.420f, 0.478f, 0.553f, 1.0f });
                const ImU32 accent = item.external
                    ? ImGui::ColorConvertFloat4ToU32({ 1.000f, 0.176f, 0.420f, 1.0f })
                    : (item.state && !checked)
                    ? muted
                    : ImGui::ColorConvertFloat4ToU32({ 0.000f, 0.831f, 1.000f, 1.0f });

                list->AddRectFilled(min, max, hovered && !item.disabled ? bg_hover : bg, 3.0f);
                list->AddRect(min, max, hovered && !item.disabled ? accent : border, 3.0f, 0, hovered && !item.disabled ? 1.25f : 1.0f);
                list->AddRectFilled(min, ImVec2(min.x + 2.0f, max.y), accent, 1.0f);

                const std::string title = item.title.empty() ? this->_title : item.title;
                list->AddText(font_subtitle, 16.0f, min + ImVec2(13.0f, detail.empty() ? 14.0f : 8.0f), text, title.c_str());
                if (!detail.empty() && (item.state || item.external))
                {
                    list->AddText(font_subtitle, 13.0f, min + ImVec2(13.0f, 27.0f), muted, detail.c_str());
                }

                if (item.state)
                {
                    const ImVec2 switch_size = ImVec2(44.0f, 22.0f);
                    const ImVec2 switch_min = ImVec2(max.x - switch_size.x - 13.0f, min.y + (row_height - switch_size.y) * 0.5f);
                    const ImVec2 switch_max = switch_min + switch_size;
                    const ImU32 switch_bg = ImGui::ColorConvertFloat4ToU32(checked
                        ? ImVec4{ 0.000f, 0.831f, 1.000f, hovered ? 0.34f : 0.24f }
                        : ImVec4{ 0.420f, 0.478f, 0.553f, hovered ? 0.30f : 0.18f });
                    const ImU32 knob = checked
                        ? ImGui::ColorConvertFloat4ToU32({ 0.000f, 1.000f, 0.529f, 1.0f })
                        : ImGui::ColorConvertFloat4ToU32({ 0.420f, 0.478f, 0.553f, 1.0f });
                    const float knob_x = checked ? switch_max.x - 11.0f : switch_min.x + 11.0f;

                    list->AddRectFilled(switch_min, switch_max, switch_bg, 11.0f);
                    list->AddRect(switch_min, switch_max, accent, 11.0f, 0, hovered ? 1.35f : 1.0f);
                    list->AddCircleFilled(ImVec2(knob_x, switch_min.y + 11.0f), hovered ? 7.0f : 6.2f, knob);
                }
                else if (!detail.empty() && !item.external)
                {
                    const ImVec2 detail_size = font_subtitle->CalcTextSizeA(12.0f, FLT_MAX, 0.0f, detail.c_str());
                    const ImVec2 badge_min = ImVec2(max.x - detail_size.x - 23.0f, min.y + 14.0f);
                    const ImVec2 badge_max = badge_min + detail_size + ImVec2(11.0f, 6.0f);
                    list->AddRect(badge_min, badge_max, item.disabled ? muted : accent, 2.0f, 0, 1.0f);
                    list->AddText(font_subtitle, 12.0f, badge_min + ImVec2(5.5f, 3.0f), item.disabled ? muted : accent, detail.c_str());
                }

                if (item.external) {
                    static auto frame = ImVec2(15, 15);
                    const ImVec2 pos = ImVec2(max.x - frame.x - 14.0f, min.y + (row_height - frame.y) * 0.5f);
                    list->AddImage(_get_texture("icon_outside_window"), pos, pos + frame, ImVec2(0, 0), ImVec2(1, 1), accent);
                }

                if (!item.tooltip.empty()) ImGui::SetItemTooltip(item.tooltip.c_str());
                if (clicked) item.action();
                i ++;

                if (item.divide_next) {
                    ImGui::Dummy(ImVec2(0.0f, 4.0f));
                }
            }

        }
        ImGui::PopFont();

        ImGui::EndPopup();
    }
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(3);
}
