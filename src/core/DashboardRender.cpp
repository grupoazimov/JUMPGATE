#include "pch.h"

#include "Dashboard.h"

extern std::unique_ptr<std::vector<std::shared_ptr<Endpoint2>>> g_endpoints;
extern std::unique_ptr<Firewall> g_firewall;
extern std::unique_ptr<Settings> g_settings;
extern std::unique_ptr<core::tunneling::Tunneling> g_tunneling;
extern std::unique_ptr<util::watcher::window::WindowWatcher> g_window_watcher;

extern ImFont* font_title;
extern ImFont* font_subtitle;
extern ImFont* font_text;

namespace {
    constexpr ImVec4 COLOR_BG{ 0.031f, 0.047f, 0.063f, 1.0f };
    constexpr ImVec4 COLOR_SURFACE{ 0.051f, 0.067f, 0.090f, 1.0f };
    constexpr ImVec4 COLOR_SURFACE_2{ 0.064f, 0.086f, 0.125f, 1.0f };
    constexpr ImVec4 COLOR_BORDER{ 0.110f, 0.137f, 0.200f, 1.0f };
    constexpr ImVec4 COLOR_TEXT{ 0.910f, 0.929f, 0.949f, 1.0f };
    constexpr ImVec4 COLOR_MUTED{ 0.420f, 0.478f, 0.553f, 1.0f };
    constexpr ImVec4 COLOR_HOT{ 1.000f, 0.176f, 0.420f, 1.0f };
    constexpr ImVec4 COLOR_ICE{ 0.000f, 0.831f, 1.000f, 1.0f };
    constexpr ImVec4 COLOR_SUCCESS{ 0.000f, 1.000f, 0.529f, 1.0f };

    ImU32 col(ImVec4 color)
    {
        return ImGui::ColorConvertFloat4ToU32(color);
    }

    void renderBackground()
    {
        ImDrawList* list = ImGui::GetWindowDrawList();
        const ImVec2 window_pos = ImGui::GetWindowPos();
        const ImVec2 window_size = ImGui::GetWindowSize();
        const ImVec2 min = window_pos;
        const ImVec2 max = window_pos + window_size;

        list->AddRectFilled(min, max, col(COLOR_BG), 4.0f);

        const ImU32 dot = ImGui::ColorConvertFloat4ToU32({ 0.91f, 0.93f, 0.95f, 0.055f });
        const float step = 28.0f;
        const float start_x = min.x + fmodf(-min.x, step);
        const float start_y = min.y + fmodf(-min.y, step);
        for (float y = start_y; y < max.y; y += step)
        {
            for (float x = start_x; x < max.x; x += step)
            {
                list->AddCircleFilled(ImVec2(x, y), 1.15f, dot);
            }
        }

        list->AddRectFilledMultiColor(
            min,
            max,
            ImGui::ColorConvertFloat4ToU32({ 1.0f, 0.176f, 0.420f, 0.08f }),
            ImGui::ColorConvertFloat4ToU32({ 0.0f, 0.831f, 1.0f, 0.05f }),
            ImGui::ColorConvertFloat4ToU32({ 0.0f, 0.831f, 1.0f, 0.02f }),
            ImGui::ColorConvertFloat4ToU32({ 1.0f, 0.176f, 0.420f, 0.02f })
        );
    }

    void renderSidebar(std::vector<std::unique_ptr<PopupButton>>& buttons)
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, COLOR_BG);
        ImGui::BeginChild("jumpgate_sidebar", ImVec2(72.0f, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            ImDrawList* list = ImGui::GetWindowDrawList();
            const ImVec2 min = ImGui::GetWindowPos();
            const ImVec2 max = min + ImGui::GetWindowSize();
            list->AddRectFilled(min, max, col(COLOR_BG), 0.0f);
            list->AddLine(ImVec2(max.x - 1.0f, min.y), ImVec2(max.x - 1.0f, max.y), col(COLOR_BORDER));

            float y = 18.0f;
            for (auto& button : buttons)
            {
                ImGui::SetCursorPos(ImVec2(19.0f, y));
                button->render();
                if (!button->getTooltip().empty())
                {
                    ImGui::SetItemTooltip(button->getTooltip().c_str());
                }
                y += 56.0f;
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    void renderHeader()
    {
        ImDrawList* list = ImGui::GetWindowDrawList();
        const ImVec2 start = ImGui::GetCursorScreenPos();
        const float width = ImGui::GetContentRegionAvail().x;
        const bool pending_rules = g_settings && g_settings->hasPendingConfigWrite();
        const auto blocked_regions = g_settings ? g_settings->getBlockedEndpointCount() : 0;

        ImGui::Dummy(ImVec2(width, 74.0f));

        list->AddText(font_subtitle, 15.0f, start + ImVec2(0.0f, 0.0f), col(COLOR_ICE), "// CONTROLE DE REGIÕES");
        list->AddText(font_title, 32.0f, start + ImVec2(0.0f, 30.0f), col(COLOR_TEXT), "JUMPGATE");

        const std::string rules = pending_rules
            ? "ALTERAÇÕES PENDENTES"
            : (std::to_string(blocked_regions) + " REGIÕES BLOQUEADAS");
        const ImVec2 rules_size = font_subtitle->CalcTextSizeA(15.0f, FLT_MAX, 0.0f, rules.c_str());
        list->AddText(font_subtitle, 15.0f, start + ImVec2(width - rules_size.x - 10.0f, 36.0f), pending_rules ? col(COLOR_HOT) : col(COLOR_SUCCESS), rules.c_str());
    }

    void syncEndpointSetting(int i)
    {
        if ((*(*g_endpoints)[i]).getBlockDesired())
        {
            (*g_settings).addBlockedEndpoint((*(*g_endpoints)[i]).getTitle());
        }
        else
        {
            (*g_settings).removeBlockedEndpoint((*(*g_endpoints)[i]).getTitle());
        }
    }

    void renderEndpoints(const ImVec2& size)
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, COLOR_BG);
        ImGui::BeginChild("endpoints_scrollable", size, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            const auto table_width = std::max(0.0f, ImGui::GetContentRegionAvail().x - 12.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(6.0f, 5.0f));
            if (ImGui::BeginTable("endpoints_grid", 3, ImGuiTableFlags_SizingStretchSame, ImVec2(table_width, 0)))
            {
                for (int i = 0; i < (*g_endpoints).size(); i++)
                {
                    ImGui::TableNextColumn();
                    if ((*(*g_endpoints)[i]).render(i))
                    {
                        syncEndpointSetting(i);
                    }
                }
                ImGui::EndTable();
            }
            ImGui::PopStyleVar();

            const float remaining_height = ImGui::GetContentRegionAvail().y;
            if (remaining_height > 34.0f)
            {
                const std::string hint = "FILAS DEMORADAS? MUDE DE REGIÃO";
                const std::string site = "azimovesports.com/jumpgate";
                const ImVec2 hint_size = font_subtitle->CalcTextSizeA(14.0f, FLT_MAX, 0.0f, hint.c_str());
                const ImVec2 site_size = font_subtitle->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, site.c_str());
                const ImVec2 start = ImGui::GetCursorScreenPos();
                const float y = start.y + std::min(44.0f, (remaining_height - hint_size.y - site_size.y - 6.0f) * 0.5f);
                ImGui::GetWindowDrawList()->AddText(
                    font_subtitle,
                    14.0f,
                    ImVec2(start.x + (table_width - hint_size.x) * 0.5f, y),
                    col(COLOR_MUTED),
                    hint.c_str()
                );

                const ImVec2 site_pos = ImVec2(start.x + (table_width - site_size.x) * 0.5f, y + hint_size.y + 6.0f);
                ImGui::SetCursorScreenPos(site_pos - ImVec2(6.0f, 3.0f));
                ImGui::InvisibleButton("jumpgate_site_link", site_size + ImVec2(12.0f, 7.0f));
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                {
                    system("start https://azimovesports.com/jumpgate");
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
                {
                    ImGui::SetItemTooltip("Abrir azimovesports.com/jumpgate");
                }
                ImGui::GetWindowDrawList()->AddText(
                    font_subtitle,
                    13.0f,
                    site_pos,
                    ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) ? col(COLOR_SUCCESS) : col(COLOR_ICE),
                    site.c_str()
                );
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    void renderStatusBar()
    {
        const bool firewall_ok = g_firewall && g_firewall->isNetworkProtected();
        const bool game_open = g_window_watcher && g_window_watcher->isActive();
        const bool pending_rules = g_settings && g_settings->hasPendingConfigWrite();
        const auto blocked_regions = g_settings ? g_settings->getBlockedEndpointCount() : 0;

        ImDrawList* list = ImGui::GetWindowDrawList();
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const float width = ImGui::GetContentRegionAvail().x;
        const float height = 38.0f;

        ImGui::Dummy(ImVec2(width, height));
        const ImVec2 min = pos;
        const ImVec2 max = pos + ImVec2(width, height);

        list->AddRectFilled(min, max, col(COLOR_BG), 0.0f);
        list->AddLine(min, ImVec2(max.x, min.y), col(COLOR_BORDER));

        struct Item {
            const char* text;
            bool ok;
        };

        const std::string rules_text = pending_rules
            ? "ALTERAÇÕES PENDENTES"
            : (std::to_string(blocked_regions) + " REGIÕES BLOQUEADAS");
        const bool latency_optimized = g_settings && g_settings->isLatencyOptimized();
        const std::string latency_text = latency_optimized ? "CONEXÃO OTIMIZADA" : "OTIMIZAÇÃO AUTOMÁTICA";
        const Item items[] = {
            { firewall_ok ? "FIREWALL ATIVO" : "FIREWALL INATIVO", firewall_ok },
            { game_open ? "OVERWATCH ABERTO" : "OVERWATCH FECHADO", game_open },
            { rules_text.c_str(), !pending_rules },
            { latency_text.c_str(), true },
        };

        float x = min.x + 16.0f;
        for (const auto& item : items)
        {
            const ImU32 status_color = item.ok ? col(COLOR_SUCCESS) : col(COLOR_HOT);
            list->AddCircleFilled(ImVec2(x, min.y + 15.0f), 3.5f, status_color);
            x += 12.0f;
            list->AddText(font_subtitle, 13.0f, ImVec2(x, min.y + 7.0f), col(COLOR_MUTED), item.text);
            x += font_subtitle->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, item.text).x + 24.0f;
        }
    }
}

void Dashboard::render()
{
    renderBackground();

    const float status_height = 38.0f;
    const ImVec2 avail = ImGui::GetContentRegionAvail();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, COLOR_BG);
    ImGui::BeginChild("jumpgate_body", ImVec2(avail.x, std::max(0.0f, avail.y - status_height)), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {
        renderSidebar(this->header_actions);
        ImGui::SameLine(0.0f, 0.0f);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, COLOR_BG);
        ImGui::BeginChild("jumpgate_matrix", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            ImGui::SetCursorPos(ImVec2(24.0f, 18.0f));
            ImGui::BeginGroup();
            {
                renderHeader();
                renderEndpoints(ImVec2(0.0f, std::max(0.0f, ImGui::GetContentRegionAvail().y - 4.0f)));
            }
            ImGui::EndGroup();
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    renderStatusBar();

    if (g_tunneling)
    {
        g_tunneling->render();
    }
}
