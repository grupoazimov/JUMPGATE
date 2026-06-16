#include "pch.h"

#include "App.h"

extern std::unique_ptr<Dashboard> g_dashboard;
extern std::unique_ptr<Firewall> g_firewall;
extern std::unique_ptr<Settings> g_settings;
extern HWND g_hwnd;

#ifdef _DEBUG
	extern std::unique_ptr<Debug> g_debug;
#endif

namespace {
	void renderMinimizeButton()
	{
		const ImVec2 window_pos = ImGui::GetWindowPos();
		const ImVec2 window_size = ImGui::GetWindowSize();
		const ImVec2 button_size = ImVec2(32.0f, 30.0f);
		const ImVec2 button_pos = ImVec2(window_pos.x + window_size.x - 70.0f, window_pos.y + 1.0f);

		ImGui::SetCursorScreenPos(button_pos);
		ImGui::InvisibleButton("##minimize_window", button_size);
		const bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
		const bool active = ImGui::IsItemActive();
		const bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);

		ImDrawList* list = ImGui::GetForegroundDrawList();
		const ImVec2 min = ImGui::GetItemRectMin();
		const ImVec2 max = ImGui::GetItemRectMax();
		const ImU32 hover_bg = ImGui::ColorConvertFloat4ToU32({ 0.063f, 0.086f, 0.125f, 1.0f });
		const ImU32 active_bg = ImGui::ColorConvertFloat4ToU32({ 0.118f, 0.227f, 0.373f, 1.0f });
		const ImU32 icon = ImGui::ColorConvertFloat4ToU32((hovered || active)
			? ImVec4{ 0.910f, 0.929f, 0.949f, 1.0f }
			: ImVec4{ 0.760f, 0.800f, 0.850f, 1.0f });

		if (hovered || active)
		{
			list->AddRectFilled(min, max, active ? active_bg : hover_bg, 2.0f);
			if (hovered)
				ImGui::SetItemTooltip("Minimizar");
		}

		const float y = min.y + 18.0f + (active ? 1.0f : 0.0f);
		list->AddLine(ImVec2(min.x + 9.0f, y), ImVec2(max.x - 9.0f, y), icon, 1.8f);

		if (clicked && g_hwnd)
		{
			HWND target_hwnd = nullptr;
			if (const auto viewport = ImGui::GetWindowViewport())
			{
				target_hwnd = static_cast<HWND>(viewport->PlatformHandle);
				if (!target_hwnd)
					target_hwnd = static_cast<HWND>(viewport->PlatformHandleRaw);
			}
			::SendMessageW(target_hwnd ? target_hwnd : g_hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
		}
	}
}

void App::render(bool* p_open) {

#ifdef _DEBUG
	if (g_settings) {
		ImGui::Begin("depuração");

		ImGui::Text(((json) (*g_settings).getAppSettings()).dump(4).c_str());
		ImGui::End();
	}
	if (g_debug) {
		(*g_debug).render();
	}
#endif

	static const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::SetNextWindowSize(ImVec2(1240, 650), ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(240, 80), ImGuiCond_Once);
	ImGui::Begin("AZIMOV JUMPGATE", p_open, window_flags);

	{
		/* draw */

		if (g_dashboard) (*g_dashboard).render();
		if (g_firewall) (*g_firewall).render();
		if (g_settings) (*g_settings).render();
	}

	renderMinimizeButton();

	ImGui::End();
}


extern ImFont* font_subtitle;

void App::renderError(std::string error) {

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
	ImGui::Begin("erro", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);


	ImGui::TextColored(ImVec4{ 1, 0.6f, 0.6f, 1 }, "o aplicativo falhou");
	ImGui::SameLine();
	ImGui::Text("::[");

	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::PushFont(font_subtitle);
	ImGui::SeparatorText("erro");
	ImGui::TextColored(ImVec4{ 1, 0.6f, 0.6f, 1 }, error.c_str());

	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::SeparatorText("ajuda");
	ImGui::TextUnformatted("AZIMOV JUMPGATE");
	ImGui::TextUnformatted("Powered by Jumpgate");
	ImGui::TextUnformatted("Edição Brasileira da Comunidade");
	ImGui::Spacing();
	ImGui::TextUnformatted("Comunidade Overwatch Brasil");
	ImGui::TextUnformatted("discord.gg/overwatchbrasil");


	/* comunidade */
	{
		static const ImVec4 color_button = { 0.345f, 0.396f, 0.949f, 1.0f };
		static const ImVec4 color_button_hover = { 0.345f, 0.396f, 0.949f, 0.9f };
		static const ImVec4 color_button_active = { 0.345f, 0.396f, 0.949f, 0.8f };

		ImGui::PushStyleColor(ImGuiCol_Button, color_button);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, color_button_active);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color_button_hover);
		{
			if (ImGui::Button("Comunidade Overwatch Brasil")) {
				system("start https://discord.gg/overwatchbrasil");
			}
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
			ImGui::SetItemTooltip("https://discord.gg/overwatchbrasil");
			ImGui::PopStyleColor(1);

		}
		ImGui::PopStyleColor(3);
	}

	ImGui::SameLine();

	ImGui::PopFont();

	ImGui::PopStyleColor(2);

	ImGui::Spacing();

	ImGui::End();
}
