#include "pch.h"

#include "Endpoint.h"

extern ImFont* font_title;
extern ImFont* font_subtitle;
extern ImFont* font_text;

namespace {
	constexpr ImVec4 COLOR_SURFACE{ 0.063f, 0.086f, 0.125f, 1.0f };
	constexpr ImVec4 COLOR_SURFACE_HOVER{ 0.077f, 0.105f, 0.153f, 1.0f };
	constexpr ImVec4 COLOR_BORDER{ 0.110f, 0.137f, 0.200f, 1.0f };
	constexpr ImVec4 COLOR_TEXT{ 0.910f, 0.929f, 0.949f, 1.0f };
	constexpr ImVec4 COLOR_MUTED{ 0.420f, 0.478f, 0.553f, 1.0f };
	constexpr ImVec4 COLOR_HOT{ 1.000f, 0.176f, 0.420f, 1.0f };
	constexpr ImVec4 COLOR_ICE{ 0.000f, 0.831f, 1.000f, 1.0f };
	constexpr ImVec4 COLOR_SUCCESS{ 0.000f, 1.000f, 0.529f, 1.0f };
	constexpr ImVec4 COLOR_WARN{ 1.000f, 0.760f, 0.220f, 1.0f };

	ImU32 col(ImVec4 color)
	{
		return ImGui::ColorConvertFloat4ToU32(color);
	}
}

bool Endpoint2::render(int i) {

	if (*(this->ping.get())) {
		const auto ping_ms = (*(this->ping.get())).value();
		if (this->ping_ms_display != ping_ms)
		{
			static const float min_delay = 9.0f;
			static const float max_delay = 90.0f;

			if (ping_ms > 0 && this->ping_ms_display <= 0)
			{
				this->ping_ms_display = ping_ms;
			}
			else {
				float param = fmin((float)std::abs(ping_ms - this->ping_ms_display) / 24.0f, 1.0f);

				if (!(ImGui::GetFrameCount() % (int)fmax(min_delay, max_delay - (max_delay * param * half_pi)) != 0)) {
					if (this->ping_ms_display < ping_ms)
						this->ping_ms_display++;
					else
						this->ping_ms_display--;

					this->ping_ms_display = std::max(0, this->ping_ms_display);
				}
			}
		}
	}

	ImGui::PushID(i);
	const ImVec2 card_size = ImVec2(ImGui::GetContentRegionAvail().x, 76.0f);
	ImGui::InvisibleButton("endpoint_card", card_size);
	const bool no_ping = this->hasNoPing();
	if (no_ping)
	{
		this->blocked_desired = true;
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
		{
			ImGui::SetItemTooltip("Servidor sem resposta. Bloqueado automaticamente para evitar conexões instáveis.");
		}
	}

	const bool action = ImGui::IsItemClicked(ImGuiMouseButton_Left) && !no_ping;
	if (action)
	{
		this->blocked_desired = !this->blocked_desired;
	}

	const bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
	const bool desired_blocked = this->blocked_desired;
	const bool unsynced = (this->blocked != this->blocked_desired);

	ImDrawList* list = ImGui::GetWindowDrawList();
	const ImVec2 min = ImGui::GetItemRectMin();
	const ImVec2 max = ImGui::GetItemRectMax();
	const ImVec2 size = ImGui::GetItemRectSize();

	const ImU32 accent = unsynced ? col(COLOR_ICE) : (desired_blocked ? col(COLOR_HOT) : col(COLOR_SUCCESS));
	const ImU32 bg = hovered ? col(COLOR_SURFACE_HOVER) : col(COLOR_SURFACE);
	const ImU32 border = unsynced ? col(COLOR_ICE) : col(COLOR_BORDER);

	list->AddRectFilled(min, max, bg, 3.0f);
	list->AddRect(min, max, border, 3.0f, 0, unsynced ? 1.5f : 1.0f);
	list->AddRectFilled(min, ImVec2(min.x + 3.0f, max.y), accent, 1.0f);

	if (unsynced)
	{
		const float stripe_step = 10.0f;
		const ImU32 stripe = ImGui::ColorConvertFloat4ToU32({ 0.0f, 0.831f, 1.0f, 0.18f });
		list->PushClipRect(min, max, true);
		for (float x = min.x - size.y; x < max.x; x += stripe_step)
		{
			list->AddLine(ImVec2(x, max.y), ImVec2(x + size.y, min.y), stripe, 1.0f);
		}
		list->PopClipRect();
	}

	const char* badge = unsynced ? "PENDENTE" : (desired_blocked ? "BLOQUEADO" : "PERMITIDO");
	const ImU32 badge_color = unsynced ? col(COLOR_ICE) : (desired_blocked ? col(COLOR_HOT) : col(COLOR_SUCCESS));
	const ImVec2 badge_size = font_subtitle->CalcTextSizeA(11.0f, FLT_MAX, 0.0f, badge);
	const ImVec2 badge_min = ImVec2(max.x - badge_size.x - 20.0f, min.y + 13.0f);
	const ImVec2 badge_max = badge_min + badge_size + ImVec2(12.0f, 7.0f);
	list->AddRect(badge_min, badge_max, badge_color, 2.0f, 0, 1.0f);
	list->AddText(font_subtitle, 11.0f, badge_min + ImVec2(6.0f, 3.5f), badge_color, badge);

	list->AddText(font_title, 26.0f, min + ImVec2(18.0f, 17.0f), col(COLOR_TEXT), this->display_title.c_str());
	list->AddText(font_subtitle, 15.0f, min + ImVec2(18.0f, 49.0f), col(COLOR_MUTED), this->description.c_str());

	if (this->pinger != nullptr || (*(this->ping)))
	{
		const auto ping = (*(this->ping)) ? (*(this->ping.get())).value() : -2;
		const std::string ping_text = ping == -2 ? "PING..." : (ping < 0 ? "SEM PING" : (std::to_string(this->ping_ms_display) + " MS"));
		const ImVec2 ping_size = font_subtitle->CalcTextSizeA(12.0f, FLT_MAX, 0.0f, ping_text.c_str());
		const ImU32 ping_color =
			ping == -2 ? col(COLOR_ICE) :
			ping < 0 ? col(COLOR_HOT) :
			ping <= 70 ? col(COLOR_SUCCESS) :
			ping <= 140 ? col(COLOR_ICE) :
			ping <= 220 ? col(COLOR_WARN) :
			col(COLOR_HOT);
		list->AddText(font_subtitle, 12.0f, ImVec2(max.x - ping_size.x - 12.0f, max.y - 18.0f), ping_color, ping_text.c_str());
	}

	ImGui::PopID();
	ImGui::Dummy(ImVec2(0.0f, 2.0f));

	return action;
}
