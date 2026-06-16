#include "pch.h"

#include "Settings.h"
#include <tlhelp32.h>


extern std::unique_ptr<std::vector<std::shared_ptr<Endpoint2>>> g_endpoints;
extern std::unique_ptr<Firewall> g_firewall;
extern std::unique_ptr<util::watcher::window::WindowWatcher> g_window_watcher;

extern ImFont* font_subtitle;

namespace {
	std::optional<DWORD> findProcessIdByExecutableName(const std::wstring& executable_name)
	{
		const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (snapshot == INVALID_HANDLE_VALUE) return std::nullopt;

		PROCESSENTRY32W entry{};
		entry.dwSize = sizeof(PROCESSENTRY32W);

		std::optional<DWORD> process_id = std::nullopt;
		BOOL has_entry = Process32FirstW(snapshot, &entry);

		while (has_entry)
		{
			if (_wcsicmp(entry.szExeFile, executable_name.c_str()) == 0)
			{
				process_id = entry.th32ProcessID;
				break;
			}

			has_entry = Process32NextW(snapshot, &entry);
		}

		CloseHandle(snapshot);
		return process_id;
	}

	bool optimizeGameProcess(HANDLE process)
	{
		const bool priority_optimized = SetPriorityClass(process, ABOVE_NORMAL_PRIORITY_CLASS) != 0;
		bool power_optimized = false;

#ifdef PROCESS_POWER_THROTTLING_CURRENT_VERSION
		PROCESS_POWER_THROTTLING_STATE power_throttling{};
		power_throttling.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
		power_throttling.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
		power_throttling.StateMask = 0;
		power_optimized = SetProcessInformation(
			process,
			ProcessPowerThrottling,
			&power_throttling,
			sizeof(power_throttling)
		) != 0;
#endif

		return priority_optimized || power_optimized;
	}
}

namespace jumpgate::settings {
	void to_json(json& j, const jumpgate_app_settings& p) {

		j = json {
			{"options", {
				{ "ping_servers", p.options.ping_servers },
				{ "tunneling", p.options.tunneling },
				{ "autosave_config", p.options.autosave_config },
			}},
			{"config", {
				{ "blocked_endpoints", p.config.blocked_endpoints },
			}},
		};

		if (p.config.tunneling_path)
		{
			j["/config/tunneling_path"_json_pointer] = p.config.tunneling_path.value();
		}
	}
	
	json strip_diff_jumpgate_app_settings(const json& j_default, const json& j) {

		json result;

		// web https://json.nlohmann.me/features/json_pointer/

		/* types that support equality */
		const std::array /*<json::json_pointer, _>*/ compare
		{
			/* bool */
			"/options/ping_servers"_json_pointer,
			"/options/tunneling"_json_pointer,
			"/options/autosave_config"_json_pointer,

			/* vector<string> */
			"/config/blocked_endpoints"_json_pointer,
		};

		for (auto& p : compare)
			if (j.contains(p) && j_default.at(p) != j.at(p))
				result[p] = j.at(p);


		/* optional values */
		const auto p = "/config/tunneling_path"_json_pointer;
		if (j.contains(p))
		{
			result[p] = j.at(p);
		}

		return result;
	}


	void from_json(const json& j, jumpgate_app_settings& p) {
		if (j.contains("/options/ping_servers"_json_pointer)) j.at("/options/ping_servers"_json_pointer).get_to(p.options.ping_servers);
		if (j.contains("/options/tunneling"_json_pointer)) j.at("/options/tunneling"_json_pointer).get_to(p.options.tunneling);
		if (j.contains("/options/autosave_config"_json_pointer)) j.at("/options/autosave_config"_json_pointer).get_to(p.options.autosave_config);

		if (j.contains("/config/blocked_endpoints"_json_pointer)) j.at("/config/blocked_endpoints"_json_pointer).get_to(p.config.blocked_endpoints);

		/* optional values */
		//if (j.contains("/config/tunneling_path"_json_pointer)) j.at("/config/tunneling_path"_json_pointer).get_to(p.config.tunneling_path);
		const auto pt = "/config/tunneling_path"_json_pointer;
		if (j.contains(pt)) {
			p.config.tunneling_path = std::make_optional<std::filesystem::path>(j.at(pt));
		}
	}
}




const jumpgate::settings::jumpgate_app_settings& Settings::getAppSettings()
{
	return this->_jumpgate_app_settings;
};

bool Settings::hasPendingConfigWrite() const
{
	return this->_waiting_for_config_write;
}

size_t Settings::getBlockedEndpointCount() const
{
	return this->_jumpgate_app_settings.config.blocked_endpoints.size();
}

bool Settings::isLatencyOptimized() const
{
	return this->_latency_optimized;
}

bool Settings::isAutosaveEnabled() const
{
	return this->_jumpgate_app_settings.options.autosave_config;
}


std::string Settings::getAllBlockedAddresses() {

	std::string result;

	if (this->_jumpgate_app_settings.config.blocked_endpoints.empty()) return result;

	std::set<std::string> blocked_servers;

	/* note this causes weird crash. */
	/*for (auto& e : this->_jumpgate_app_settings.config.blocked_endpoints)
	{
		auto& endpoint = this->__ow2_endpoints.at(e);

		for (auto s : endpoint.blocked_servers)
		{
			blocked_servers.insert(s);
		}
	}

	for (auto& s : blocked_servers)
	{
		auto& server = this->__ow2_servers.at(s);
		result += server.block;
		result += ',';
	}*/

	//if (!result.empty())
	//{
	//	result = result.substr(0, result.length() - 1);
	//}

	for (auto& e : this->_jumpgate_app_settings.config.blocked_endpoints)
	{
		if (this->__ow2_endpoints.contains(e)) {

			auto& endpoint = this->__ow2_endpoints.at(e);

			for (auto s : endpoint.blocked_servers)
			{
				auto& server = this->__ow2_servers.at(s);
				result += server.block;
				result += ',';
			}
		}
	}

	if (!result.empty()) {
		result.pop_back();
	}

	// throws errors
	//println("blocking {}", result);

	return result;

}



std::optional<json> Settings::readStoragePatch__win_firewall() {

#ifdef _DEBUG
	util::timer::Timer timer("readStoragePatch__win_firewall");
#endif

	std::optional<json> result = std::nullopt;

	auto loaded_settings = (*g_firewall).tryFetchSettingsFromFirewall();
	if (loaded_settings)
	{

		try {
			//result = std::make_optional<json>(loaded_settings.value());
			result = std::make_optional<json>(json::from_msgpack(loaded_settings.value()));
			println("patch carregado: {}", result.value().dump(4));
		}
		catch (json::exception& e) {
			println("erro de json: {}", e.what());
		}
	}

	return result;
}

void Settings::tryLoadSettingsFromStorage() {
	auto loaded_settings = this->readStoragePatch();
	if (loaded_settings) {

		try {
			json settings = this->__default_jumpgate_app_settings;

			//settings.merge_patch(loaded_settings.value());

			/* merge obects: false */
			settings.update(loaded_settings.value(), false);

			this->_jumpgate_app_settings = settings;
		}
		catch (json::exception& e) {
			println("erro de json em tryLoadSettingsFromStorage(): {}", e.what());
		}
	}
}


/* note: windows firewall has a 1024 description */
void Settings::tryWriteSettingsToStorage(bool force) {

#ifdef _DEBUG
	util::timer::Timer timer("tryWriteSettingsToStorage");
#endif

	const auto game_open = g_window_watcher && (*g_window_watcher).isActive();
	if (game_open && !force)
	{
		//this->_waiting_for_config_write = true;
		auto any = false;
		for (auto& e : (*g_endpoints)) {
			if ((*e).getBlockDesired() != (*e)._getBlockedState()) { any = true; };
		}
		this->_waiting_for_config_write = any;
	}
	else if (force && this->_waiting_for_config_write)
	{
		this->_waiting_for_config_write = false;
	}

	if (!this->_waiting_for_config_write || force)
	{
		// calculate diff
		/*auto diff = json::diff(this->__default_jumpgate_app_settings, this->_jumpgate_app_settings);
		println("diff: {}", diff.dump(4));*/

		auto settings_for_storage = this->_jumpgate_app_settings;
		if (!settings_for_storage.options.autosave_config)
		{
			settings_for_storage.config.blocked_endpoints.clear();
		}

		json stripped = jumpgate::settings::strip_diff_jumpgate_app_settings(this->__default_jumpgate_app_settings, settings_for_storage);

		if (stripped.is_null())
		{
			stripped = this->__default_jumpgate_app_settings;
		}

		// throws error
		//println("writing: {}", stripped.dump(4));

		/* note: diff calcuated in to_json defined above */
		auto packed = json::to_msgpack(stripped);
		std::string s(packed.begin(), packed.end());

		(*g_firewall).tryWriteSettingsToFirewall(s, this->getAllBlockedAddresses(), this->getAppSettings().options.tunneling ? this->getAppSettings().config.tunneling_path : std::nullopt);


		// TODO if not failed
		for (auto& endpoint : (*g_endpoints)) {
			(*endpoint)._setBlockedState(this->_jumpgate_app_settings.config.blocked_endpoints.contains((*endpoint).getTitle()));
		}


		//std::basic_string_view<uint8_t> packed_sv (packed.data(), packed.size());
		//std::string packed_s{ packed_sv };
		//auto s = std::string_view<uint8_t>(packed);

		//(*g_firewall).tryWriteSettingsToFirewall(s);

		//printf("packed: <%s>\n", s.c_str());
	}

	else
	{
		println("aguardando o jogo fechar");
	}

}

//json mergeSettings(const json& a, const json& b) {
//
//	println("a {}", a.dump(4));
//	println("b {}", b.dump(4));
//
//	throw std::runtime_error("xd");
//
//	/* calculate diff */
//	json merge_patch = a;
//
//	//merge_patch.merge_patch(b);
//	for (auto& b : a.flatten()) {
//		println("ac: {}", b.dump(4));
//	}
//
//	println("merge: {}", merge_patch.dump(4));
//
//	//a.merge_patch(b);
//
//	return a;
//}



std::optional<json> Settings::readStoragePatch() {
	return this->readStoragePatch__win_firewall();
}


Settings::Settings() {

	if (!g_endpoints) throw std::runtime_error("as configurações dependem dos endpoints.");
	if (!g_firewall) throw std::runtime_error("as configurações dependem do firewall.");


	this->tryLoadSettingsFromStorage();
	this->_jumpgate_app_settings.options.ping_servers = true;

	/*
		1. grab settings from firewall manager
		2. if failed, show failed and use default settings.
	
	*/

	(*g_endpoints).reserve(this->__ow2_endpoints.size());

	for (auto& [key, e] : this->__ow2_endpoints)
	{
		auto blocked = this->_jumpgate_app_settings.config.blocked_endpoints.contains(key);

		(*g_endpoints).push_back(std::make_shared<Endpoint2>(
			key,
			e.description,
			e.ip_ping,
			blocked,
			e.display_title,
			this->_jumpgate_app_settings.options.ping_servers
		));
	}

	if (!this->_jumpgate_app_settings.options.autosave_config)
	{
		this->tryWriteSettingsToStorage(true);
	}


	/*std::cout << "__ow2_ranges: " << __ow2_servers.dump(4) << std::endl;
	std::cout << "__ow2_ranges: " << __ow2_ranges.dump(4) << std::endl;
	std::cout << "__ow2_endpoints: " << __ow2_endpoints.dump(4) << std::endl;
	std::cout << "__default: " << __default.dump(4) << std::endl;*/

}

Settings::~Settings() {


	printf("destrutor");
}

void Settings::unblockAll() {
	for (auto& endpoint : (*g_endpoints)) {
		(*endpoint).setBlockDesired(false);
	}
	this->_auto_blocked_no_ping_endpoints.clear();
	this->_jumpgate_app_settings.config.blocked_endpoints.clear();
	this->tryWriteSettingsToStorage(true); // force
}

bool Settings::blockUnresponsiveEndpoints() {
	if (!g_endpoints) return false;

	bool changed = false;

	for (auto& endpoint : (*g_endpoints))
	{
		const auto title = endpoint->getTitle();
		if (!this->__ow2_endpoints.contains(title)) continue;

		if (endpoint->hasNoPing())
		{
			endpoint->setBlockDesired(true);

			if (!this->_jumpgate_app_settings.config.blocked_endpoints.contains(title))
			{
				this->_jumpgate_app_settings.config.blocked_endpoints.insert(title);
				this->_auto_blocked_no_ping_endpoints.insert(title);
				changed = true;
			}

			continue;
		}

		if (this->_auto_blocked_no_ping_endpoints.erase(title) > 0)
		{
			endpoint->setBlockDesired(false);

			if (this->_jumpgate_app_settings.config.blocked_endpoints.erase(title) > 0)
			{
				changed = true;
			}
		}
	}

	if (changed)
	{
		this->tryWriteSettingsToStorage();
	}

	return changed;
}

bool Settings::applyGameLatencyOptimization(bool notify_user) {
	std::vector<std::wstring> executable_names{ L"Overwatch.exe" };

	if (this->_jumpgate_app_settings.config.tunneling_path)
	{
		const auto configured_name = this->_jumpgate_app_settings.config.tunneling_path->filename().wstring();
		if (!configured_name.empty())
		{
			executable_names.insert(executable_names.begin(), configured_name);
		}
	}

	std::optional<DWORD> process_id = std::nullopt;
	for (const auto& executable_name : executable_names)
	{
		process_id = findProcessIdByExecutableName(executable_name);
		if (process_id) break;
	}

	if (!process_id)
	{
		this->_latency_optimized = false;
		this->_latency_optimized_process_id = std::nullopt;
		if (notify_user)
		{
			MessageBoxW(
				nullptr,
				L"Abra o Overwatch e tente novamente.\n\nA otimização de latência precisa encontrar o processo do jogo em execução.",
				L"AZIMOV JUMPGATE",
				MB_ICONINFORMATION | MB_OK
			);
		}
		return false;
	}

	const HANDLE process = OpenProcess(PROCESS_SET_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process_id.value());
	if (!process)
	{
		this->_latency_optimized = false;
		this->_latency_optimized_process_id = std::nullopt;
		if (notify_user)
		{
			MessageBoxW(
				nullptr,
				L"Não foi possível acessar o processo do Overwatch.\n\nExecute o Jumpgate como administrador e tente novamente.",
				L"AZIMOV JUMPGATE",
				MB_ICONWARNING | MB_OK
			);
		}
		return false;
	}

	const bool optimized = optimizeGameProcess(process);
	CloseHandle(process);

	this->_latency_optimized = optimized;
	this->_latency_optimized_process_id = optimized ? process_id : std::nullopt;

	if (notify_user)
	{
		MessageBoxW(
			nullptr,
			optimized
				? L"Otimização aplicada.\n\nO Overwatch foi priorizado no Windows para ajudar a reduzir travamentos e oscilações de latência."
				: L"Não foi possível aplicar a otimização de latência ao Overwatch.",
			L"AZIMOV JUMPGATE",
			optimized ? MB_ICONINFORMATION | MB_OK : MB_ICONWARNING | MB_OK
		);
	}

	return optimized;
}

bool Settings::optimizeGameLatency() {
	return this->applyGameLatencyOptimization(true);
}

void Settings::toggleOptionPingServers() {
	this->_jumpgate_app_settings.options.ping_servers = !this->_jumpgate_app_settings.options.ping_servers;
	this->tryWriteSettingsToStorage();

	for (auto& e : *g_endpoints) {
		if (this->_jumpgate_app_settings.options.ping_servers)
		{
			e->start_pinging();
		}
		else
		{
			e->stop_pinging();
		}
	}
}

void Settings::toggleOptionTunneling() {
	this->_jumpgate_app_settings.options.tunneling = !this->_jumpgate_app_settings.options.tunneling;
	this->tryWriteSettingsToStorage();
}

void Settings::toggleOptionAutosave() {
	this->_jumpgate_app_settings.options.autosave_config = !this->_jumpgate_app_settings.options.autosave_config;
	this->tryWriteSettingsToStorage(true);
}

void Settings::setConfigTunnelingPath(std::optional<std::filesystem::path> path)
{
	this->_jumpgate_app_settings.config.tunneling_path = path;
	this->tryWriteSettingsToStorage();
}


// todo std::unordered_set
void Settings::addBlockedEndpoint(std::string endpoint_title) {
	this->_auto_blocked_no_ping_endpoints.erase(endpoint_title);

	auto [_position, hasBeenInserted] = this->_jumpgate_app_settings.config.blocked_endpoints.insert(endpoint_title);

	if (hasBeenInserted)
	{
		this->tryWriteSettingsToStorage();
	}

}

// todo std::unordered_set
void Settings::removeBlockedEndpoint(std::string endpoint_title) {
	this->_auto_blocked_no_ping_endpoints.erase(endpoint_title);

	auto const num_removed = this->_jumpgate_app_settings.config.blocked_endpoints.erase(endpoint_title);

	if (num_removed > 0)
	{
		this->tryWriteSettingsToStorage();
	}

}

void Settings::render() {
	this->blockUnresponsiveEndpoints();

	if (g_window_watcher) {

		const auto game_open = (*g_window_watcher).isActive();

		// pause config writes when game is open
		if (game_open) {
			//this->_waiting_for_config_write = true;
			const auto now = std::chrono::steady_clock::now();
			const auto waiting_to_retry =
				this->_last_latency_optimization_attempt.time_since_epoch().count() > 0 &&
				now - this->_last_latency_optimization_attempt < 3s;

			if (!this->_latency_optimized && !waiting_to_retry)
			{
				this->_last_latency_optimization_attempt = now;
				this->applyGameLatencyOptimization(false);
			}
		}

		// trigger a write when game is closed
		else
		{
			this->_latency_optimized = false;
			this->_latency_optimized_process_id = std::nullopt;

			if (this->_waiting_for_config_write)
			{
				this->_waiting_for_config_write = false;
				this->tryWriteSettingsToStorage();
			}
		}

	}
}

void Settings::renderWaitingStatus()
{
	if (g_window_watcher) {

		if (this->_waiting_for_config_write)
		{
			//static auto& style = ImGui::GetStyle();
			ImDrawList* list = ImGui::GetWindowDrawList();

			static const ImU32 white = ImGui::ColorConvertFloat4ToU32({ 1, 1, 1, 1 });

			static const auto color = ImGui::ColorConvertFloat4ToU32({ .4f, .4f, .4f, 1.0f });
			// static const auto color_2 = ImGui::ColorConvertFloat4ToU32({ 0, 0, 0, 0.6f });
			static const std::string text = "REINICIE O JOGO PARA APLICAR";
			//static const std::string text = "GAME SHUT DOWN REQUIRED";

			static const auto font = font_subtitle;
			static const auto font_size = font->CalcTextSizeA(font_subtitle->FontSize, FLT_MAX, 0.0f, text.c_str());


			ImGui::Dummy({ ImGui::GetContentRegionAvail().x, font_size.y + 16 });

			list->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), color, 5.0f);

			{
				static const auto color = ImGui::ColorConvertFloat4ToU32({ 1, 1, 1, 0.09f });
				const auto pos = ImGui::GetItemRectMin();

				static const auto image = _get_image("background_diagonal");

				list->PushClipRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), true);
				list->AddImage(image.texture, pos, pos + ImVec2((float)image.width, (float)image.height), ImVec2(0, 0), ImVec2(1, 1), color);
				list->PopClipRect();
			}

			const auto pos = ImGui::GetItemRectMin() + ImVec2((ImGui::GetItemRectSize().x - font_size.x) / 2, 8 - 2);
			list->AddText(font_subtitle, font_subtitle->FontSize, pos, white, text.c_str());

		}

		/*static auto& style = ImGui::GetStyle();
		ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
		ImGui::TextWrapped("Click Unblock if not connecting");
		ImGui::PopStyleColor();*/
	}

	else {
		ImGui::TextDisabled("Não bloqueie servidores com o jogo aberto");
	}
}
