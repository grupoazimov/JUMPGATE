#include "pch.h"
#include "Tunneling.h"


extern std::unique_ptr<Firewall> g_firewall;
extern std::unique_ptr<Settings> g_settings;

extern ImFont* font_subtitle;

namespace
{
	const ImVec4 color_popup_bg{ 0.051f, 0.067f, 0.090f, 1.0f };
	const ImVec4 color_border{ 0.110f, 0.137f, 0.200f, 1.0f };
	const ImVec4 color_text{ 0.910f, 0.929f, 0.949f, 1.0f };
	const ImVec4 color_muted{ 0.420f, 0.478f, 0.553f, 1.0f };
	const ImVec4 color_row{ 0.063f, 0.086f, 0.125f, 0.78f };
	const ImVec4 color_row_hover{ 0.078f, 0.108f, 0.157f, 1.0f };
	const ImVec4 color_row_active{ 0.118f, 0.227f, 0.373f, 1.0f };
	const ImVec4 color_ice{ 0.000f, 0.831f, 1.000f, 1.0f };
	const ImVec4 color_hot{ 1.000f, 0.176f, 0.420f, 1.0f };

	float tunnelingPopupAnimationProgress(double opened_at)
	{
		if (opened_at <= 0.0)
			return 1.0f;

		float t = static_cast<float>((ImGui::GetTime() - opened_at) / 0.42);
		if (t < 0.0f)
			t = 0.0f;
		if (t > 1.0f)
			t = 1.0f;

		const float inv = 1.0f - t;
		return 1.0f - inv * inv * inv;
	}

	void setupTunnelingPopup(float width, double opened_at)
	{
		const ImGuiViewport* parent_viewport = ImGui::GetWindowViewport();
		const ImVec2 center = parent_viewport->GetCenter();
		const float progress = tunnelingPopupAnimationProgress(opened_at);
		const float animated_width = width - (84.0f * (1.0f - progress));
		const float animated_offset_y = 72.0f * (1.0f - progress);

		ImGuiWindowClass popup_class;
		popup_class.ParentViewportId = parent_viewport->ID;
		popup_class.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoTaskBarIcon;
		ImGui::SetNextWindowClass(&popup_class);
		ImGui::SetNextWindowViewport(parent_viewport->ID);
		ImGui::SetNextWindowPos(center + ImVec2(0.0f, animated_offset_y), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(animated_width, 0.0f), ImGuiCond_Always);
	}

	void pushTunnelingPopupStyle(double opened_at)
	{
		(void)opened_at;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 14.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 6.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, color_popup_bg);
		ImGui::PushStyleColor(ImGuiCol_Border, color_border);
		ImGui::PushStyleColor(ImGuiCol_Text, color_text);
		ImGui::PushStyleColor(ImGuiCol_TextDisabled, color_muted);
	}

	void popTunnelingPopupStyle()
	{
		ImGui::PopStyleColor(4);
		ImGui::PopStyleVar(3);
	}

	void renderTunnelingInfoRow(const char* title, const char* detail)
	{
		ImDrawList* list = ImGui::GetWindowDrawList();
		const ImVec2 row_size = ImVec2(ImGui::GetContentRegionAvail().x, 50.0f);

		ImGui::Dummy(row_size);
		const ImVec2 min = ImGui::GetItemRectMin();
		const ImVec2 max = ImGui::GetItemRectMax();

		const ImU32 text = ImGui::ColorConvertFloat4ToU32(color_text);
		const ImU32 muted = ImGui::ColorConvertFloat4ToU32(color_muted);
		const ImU32 accent = ImGui::ColorConvertFloat4ToU32({ 0.000f, 0.831f, 1.000f, 0.86f });

		list->AddRectFilled(ImVec2(min.x, min.y + 4.0f), ImVec2(min.x + 2.0f, max.y - 4.0f), accent, 1.0f);
		list->AddText(font_subtitle, 16.0f, min + ImVec2(13.0f, 7.0f), text, title);
		list->AddText(font_subtitle, 13.0f, min + ImVec2(13.0f, 28.0f), muted, detail);
	}

	bool renderTunnelingAction(const char* id, const char* title, const ImVec2& size, bool primary = false, bool disabled = false)
	{
		ImDrawList* list = ImGui::GetWindowDrawList();

		ImGui::InvisibleButton(id, size);
		const bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
		const bool active = ImGui::IsItemActive();
		const bool clicked = !disabled && ImGui::IsItemClicked(ImGuiMouseButton_Left);
		const ImVec2 min = ImGui::GetItemRectMin();
		const ImVec2 max = ImGui::GetItemRectMax();

		const ImVec4 primary_bg = active
			? ImVec4{ 0.000f, 0.365f, 0.475f, 1.0f }
			: hovered
			? ImVec4{ 0.000f, 0.290f, 0.390f, 1.0f }
			: ImVec4{ 0.063f, 0.086f, 0.125f, 1.0f };
		const ImVec4 secondary_bg = active
			? color_row_active
			: hovered
			? color_row_hover
			: ImVec4{ 0.071f, 0.094f, 0.133f, 1.0f };
		const ImVec4 bg_color = disabled ? ImVec4{ color_row.x, color_row.y, color_row.z, 0.45f } : (primary ? primary_bg : secondary_bg);
		const ImU32 bg = ImGui::ColorConvertFloat4ToU32(bg_color);
		const ImU32 border = ImGui::ColorConvertFloat4ToU32(disabled ? color_border : (primary ? color_ice : color_border));
		const ImU32 text = ImGui::ColorConvertFloat4ToU32(disabled
			? color_muted
			: primary
			? color_text
			: color_text);

		list->AddRectFilled(min, max, bg, 3.0f);
		list->AddRect(min, max, border, 3.0f, 0, (hovered || primary) && !disabled ? 1.25f : 1.0f);
		if (primary && !disabled)
			list->AddRectFilled(min, ImVec2(min.x + 2.0f, max.y), ImGui::ColorConvertFloat4ToU32(color_ice), 1.0f);

		const ImVec2 text_size = font_subtitle->CalcTextSizeA(17.0f, FLT_MAX, 0.0f, title);
		const ImVec2 text_pos = ImVec2(
			min.x + (size.x - text_size.x) * 0.5f,
			min.y + (size.y - text_size.y) * 0.5f + (active ? 1.0f : 0.0f)
		);
		list->AddText(font_subtitle, 17.0f, text_pos, text, title);

		return clicked;
	}

	bool renderTunnelingPathOption(const char* id, const std::string& path, bool selected)
	{
		ImDrawList* list = ImGui::GetWindowDrawList();
		const ImVec2 row_size = ImVec2(ImGui::GetContentRegionAvail().x, 42.0f);

		ImGui::InvisibleButton(id, row_size);
		const bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
		const bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
		const ImVec2 min = ImGui::GetItemRectMin();
		const ImVec2 max = ImGui::GetItemRectMax();

		const ImU32 bg = ImGui::ColorConvertFloat4ToU32(hovered ? color_row_hover : color_row);
		const ImU32 border = ImGui::ColorConvertFloat4ToU32(selected || hovered ? color_ice : color_border);
		const ImU32 text = ImGui::ColorConvertFloat4ToU32(selected ? color_text : color_muted);
		const ImU32 accent = ImGui::ColorConvertFloat4ToU32(selected ? color_ice : color_muted);

		list->AddRectFilled(min, max, bg, 3.0f);
		list->AddRect(min, max, border, 3.0f, 0, selected || hovered ? 1.25f : 1.0f);
		list->AddRectFilled(min, ImVec2(min.x + 2.0f, max.y), accent, 1.0f);
		const ImVec2 text_pos = min + ImVec2(13.0f, 12.0f);
		const ImVec4 clip_rect{ text_pos.x, min.y, max.x - 12.0f, max.y };
		list->AddText(font_subtitle, 15.0f, text_pos, text, path.c_str(), nullptr, 0.0f, &clip_rect);

		if (hovered)
			ImGui::SetItemTooltip(path.c_str());

		return clicked;
	}
}


namespace core::tunneling
{
	core::tunneling::Tunneling::Tunneling()
	{
#ifdef _DEBUG
		util::timer::Timer timer("core::tunneling::Tunneling::Tunneling");
#endif

		if (!g_firewall) throw std::runtime_error("o tunelamento depende do firewall.");
		if (!g_settings) throw std::runtime_error("o tunelamento depende das configurações.");

		/* if tunneling is enabled but no path defined, attempt automatic tunneling */
		if (g_settings->getAppSettings().options.tunneling && !g_settings->getAppSettings().config.tunneling_path.has_value())
		{
			const auto possible_paths = this->_queryFirewallForPossibleExePaths("Overwatch Application");

			/* if there's only one good option, set path */
			if (possible_paths.size() == 1)
			{
				const auto path = *(possible_paths.begin());
				g_settings->setConfigTunnelingPath(std::make_optional(path));
			}

			/* otherwise open list */
		}

		//wprintf(g_settings->getAppSettings().config.tunneling_path.value_or(TEXT("NONE")).c_str());
	}

	std::set<std::string> core::tunneling::Tunneling::_queryFirewallForPossibleExePaths(std::string rule_name)
	{
#ifdef _DEBUG
		util::timer::Timer timer("core::tunneling::Tunneling::_queryFirewallForPossibleExePathsUTF8Encoded");
#endif
		std::set<std::string> result;
		{
			util::win_firewall::forFirewallRulesWithName(rule_name, [&result](const CComPtr<INetFwRule>& FwRule, const CComPtr<INetFwRules>& rules) {

				CComBSTR application_name;
				if (SUCCEEDED(FwRule->get_ApplicationName(&application_name)) && application_name)
				{

					std::wstring ws(application_name, SysStringLen(application_name));

					// ws.includes '_retail_' ??

					//if is valid
					std::filesystem::path path (ws);
					if (std::filesystem::exists(path))
					{
						//if was used in the last 30 days?
						//const auto write_time = std::filesystem::last_write_time(ws);
						result.insert(path.string());
					}

				}
			});
		}

		/* test multiple options */
		/*result.insert(("S:\\Overwatch\\_beta_\\Overwatch.exe"));
		result.insert(("C:\\Program Files (x86)\\Overwatch\\Overwatch.exe"));
		result.insert(("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Overwatch\\Overwatch.exe"));*/

		/* test no options */
		/*std::set<std::string> h;
		return h;*/

		return result;
	}

	void core::tunneling::Tunneling::requestConfiguration()
	{
		this->_configuration_dismissed = false;
		this->_configuration_requested = true;
		this->_configuration_opened_at = 0.0;
		this->_path_picker_opened_at = 0.0;
	}

	void core::tunneling::Tunneling::render()
	{
#ifdef _DEBUG
		ImGui::Begin("depuração");
		if (ImGui::CollapsingHeader("tunelamento", ImGuiTreeNodeFlags_None))
		{
			if (g_settings->getAppSettings().config.tunneling_path)
			{
				ImGui::Text("caminho: %s", g_settings->getAppSettings().config.tunneling_path.value().c_str());
			}
			else
			{
				ImGui::Text("CAMINHO NÃO DEFINIDO");
			}

			{
				if (ImGui::Button("IMPRIMIR ÚNICOS", { ImGui::GetContentRegionAvail().x, 0 })) {
					auto x = this->_queryFirewallForPossibleExePaths("Overwatch Application");

					for (auto& f : x)
					{
						std::println("{}", f);
					}
				}
			}

			{
				if (ImGui::Button("definir caminho do tunelamento", { ImGui::GetContentRegionAvail().x, 0 })) {
					auto path_wstring = util::win_filesystem::prompt_file();
					if (path_wstring) {

						//std::string path_utf8encoded = util::utf8::utf8_encode(path_wstring.value());
						//std::println("{}", path_utf8encoded);

						//wprintf(path_wstring.value().c_str());
						//wprintf(util::utf8::utf8_decode(path_utf8encoded).c_str());

						//g_settings->setConfigTunnelingPath(std::make_optional(path_utf8encoded));
						g_settings->setConfigTunnelingPath(std::make_optional(path_wstring.value()));
					}
					else
					{
						std::println("falhou");
					}
				}
				ImGui::SetItemTooltip("a fazer");

				if (ImGui::Button("limpar caminho do tunelamento", { ImGui::GetContentRegionAvail().x, 0 })) {
					g_settings->setConfigTunnelingPath(std::nullopt);
				}
			}
		}
		ImGui::End();
#endif

		static const auto list = ImGui::GetWindowDrawList();
		static const auto& style = ImGui::GetStyle();


		static const std::string popup_name { "tunelamento" };
		static const std::string popup_name_continue { "localizar Overwatch.exe" };

		const bool options_tunneling = g_settings->getAppSettings().options.tunneling;
		const bool config_tunneling_path = g_settings->getAppSettings().config.tunneling_path.has_value();

		/* if tunneling popup was ignored, unignore if tunneling is toggled again*/
		static bool prev_options_tunneling { options_tunneling };
		if (options_tunneling && !prev_options_tunneling)
		{
			this->_configuration_dismissed = false;
		}
		prev_options_tunneling = options_tunneling;


		const bool tunneling_active = options_tunneling && config_tunneling_path;

		/* open configuration if tunneling is enabled but there is no path defined */
		const bool should_open_configuration =
			options_tunneling &&
			!config_tunneling_path &&
			(this->_configuration_requested || !this->_configuration_dismissed) &&
			!ImGui::IsPopupOpen(popup_name.c_str()) &&
			!ImGui::IsWindowAppearing();

		if (should_open_configuration)
		{
			ImGui::OpenPopup(popup_name.c_str());
			this->_configuration_opened_at = ImGui::GetTime();
			this->_configuration_requested = false;
		}

		/* configuration popup */
		{
			setupTunnelingPopup(540.0f, this->_configuration_opened_at);
			pushTunnelingPopupStyle(this->_configuration_opened_at);
			bool configuration_open = true;
			if (ImGui::BeginPopupModal(popup_name.c_str(), &configuration_open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
			{
				renderTunnelingInfoRow("Bloqueio por aplicativo", "Bloqueia servidores apenas para o Overwatch.exe.");
				renderTunnelingInfoRow("Mais controle", "Evita bloquear outros jogos e aplicativos no Windows.");
				renderTunnelingInfoRow("Configuração rápida", "Localize o Overwatch.exe para ativar o tunelamento.");
				ImGui::Dummy(ImVec2(0.0f, 4.0f));

				const auto n_buttons{ 2 };
				const ImVec2 button{ (ImGui::GetContentRegionAvail().x - 8.0f * (n_buttons - 1)) / n_buttons, 48.0f };

				/* choice 00 */
				{
					if (renderTunnelingAction("##skip_tunneling_setup", "Pular por enquanto", button)) {
						this->_configuration_dismissed = true;
						ImGui::CloseCurrentPopup();
					}
				}

				ImGui::SameLine(0.0f, 8.0f);

				/* choice 02 */
				if (renderTunnelingAction("##continue_tunneling_setup", "Continuar", button, true)) {
					ImGui::OpenPopup(popup_name_continue.c_str());
					this->_path_picker_opened_at = ImGui::GetTime();
				}

				/* configuration popup continued (part 2) */
				{
					setupTunnelingPopup(760.0f, this->_path_picker_opened_at);
					bool path_picker_open = true;
					bool close_configuration_after_path_picker = false;
					if (ImGui::BeginPopupModal(popup_name_continue.c_str(), &path_picker_open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
					{

						static std::optional<std::string> error_text{ std::nullopt };

						static auto possible_paths{ this->_queryFirewallForPossibleExePaths("Overwatch Application") };
						static int selected{ possible_paths.size() > 0 ? 0 : -1 };

						//ImGui::Text("selected %d", selected);

						renderTunnelingInfoRow("Escolha o executável", "Selecione o Overwatch.exe correto nos arquivos do computador.");

						int n = 0;
						for (auto& p : possible_paths) {
							if (renderTunnelingPathOption(("##tunneling_path_" + std::to_string(n)).c_str(), p, selected == n))
							{
								selected = n;
							}
							n++;
						}

						//std::println("file exists: {}", std::filesystem::exists("S:\\Overwatch\\_retail_\\Overwatch.exe") ? "true" : "false");
						//std::println("file exists: {}", std::filesystem::exists("S:\\overwatch\\_retail_\\overwatch.exe") ? "true" : "false");

						//wprintf(L"C:\\Users\\stormy\\AppData\\Local\\Temp\\べてのファ 況ロ");

						if (renderTunnelingAction("##add_tunneling_path", possible_paths.size() > 0 ? "Adicionar outro..." : "Adicionar caminho...", ImVec2(ImGui::GetContentRegionAvail().x, 48.0f), true))
						{
							auto path = util::win_filesystem::prompt_file();
							if (path) {

								std::println("{}", path.value().string());

								//wprintf(path_wstring.value().c_str());
								//wprintf(util::utf8::utf8_decode(path_utf8encoded).c_str());

								//g_settings->setConfigTunnelingPath(std::make_optional(path_utf8encoded));

								const auto size_before = possible_paths.size();
								possible_paths.insert(path.value().string());
								if (possible_paths.size() != size_before)
								{
									selected = std::max(0, (int)possible_paths.size() - 1);
								};
								error_text.reset();
							}
							else
							{
								error_text = std::make_optional("Não foi possível adicionar outro arquivo.");
							}
						}

						if (error_text)
						{
							ImGui::PushStyleColor(ImGuiCol_Text, (ImU32)ImColor::HSV(1.0f - ((0 + 1) / 32.0f), 0.5f, 1.0f, 1.0f));
							//ImGui::Indent();
							ImGui::TextUnformatted(error_text.value().c_str());
							ImGui::PopStyleColor();
						}

						ImGui::Spacing();

						{
							const auto n_buttons{ 2 };
							const ImVec2 button{ (ImGui::GetContentRegionAvail().x - 8.0f * (n_buttons - 1)) / n_buttons, 48.0f };

							/* choice 00 */
							{
								if (renderTunnelingAction("##skip_tunneling_path", "Pular por enquanto", button)) {
									this->_configuration_dismissed = true;
									close_configuration_after_path_picker = true;
									ImGui::CloseCurrentPopup();
								}
							}

							ImGui::SameLine(0.0f, 8.0f);

							/* choice 02 */

							if (renderTunnelingAction("##finish_tunneling_path", "Concluído", button, true, selected == -1)) {

								g_settings->setConfigTunnelingPath(*std::next(possible_paths.begin(), selected));
								this->_configuration_dismissed = true;
								close_configuration_after_path_picker = true;
								ImGui::CloseCurrentPopup();
							}
						}

						ImGui::EndPopup();
					}
					if (!path_picker_open)
					{
						this->_configuration_dismissed = true;
						close_configuration_after_path_picker = true;
					}
					if (close_configuration_after_path_picker)
					{
						ImGui::CloseCurrentPopup();
					}
				}

				ImGui::EndPopup();
			}
			if (!configuration_open)
			{
				this->_configuration_dismissed = true;
			}
			popTunnelingPopupStyle();
		}
	}
}
