#include "pch.h"

#include "core/Dashboard.h"
#include "core/Update.h"

extern std::unique_ptr<Settings> g_settings;
extern std::unique_ptr<core::tunneling::Tunneling> g_tunneling;

Dashboard::Dashboard()
{

	if (!g_settings) throw std::runtime_error("o painel depende das configurações.");

	auto i = 1;

	this->header_actions.push_back(std::make_unique<PopupButton>("Comunidade", "icon_heart", std::move(std::vector<Action>({
		{
			.title = "Comunidade Overwatch Brasil",
			.description = "discord.gg/overwatchbrasil",
			.action = []() { system("start https://discord.gg/overwatchbrasil"); },
			.external = true,
		},
	})), i++, "Comunidade"));

	this->header_actions.push_back(std::make_unique<PopupButton>("Configurações", "icon_options", std::move(std::vector<Action>({
		{
			.title = "Ping dos Servidores",
			//.description = "constantly",
			.description = "",
			.tooltip = "padrão: ativado\n\n(visual) estima a latência enviando ping aos servidores:\n\tativado - a cada poucos segundos\n\tdesativado - nunca",
			.action = [this]() { (*g_settings).toggleOptionPingServers(); },
			.state = [this]() { return (*g_settings).getAppSettings().options.ping_servers; },
			.external = false,

			//.disabled = true,
			.divide_next = true,
		},
		{
			.title = "Tunelamento",
			.tooltip = "padrão: ativado\n\nO tunelamento permite bloquear servidores\npor aplicativo em vez de bloquear globalmente.\n\nIsso evita que servidores de outros jogos\ne aplicativos sejam bloqueados sem querer.\n\nativado - bloqueia tráfego apenas para Overwatch.exe\ndesativado - bloqueia tráfego globalmente neste dispositivo",
			.action = [this]() { (*g_settings).toggleOptionTunneling(); },
			.state = [this]() { return (*g_settings).getAppSettings().options.tunneling; },
			.external = false,
		},
		{
			.title = "Autosave",
			.description = "",
			.tooltip = "padrão: ativado\n\nSalva automaticamente as regiões escolhidas para restaurar a sessão anterior ao abrir o Jumpgate.",
			.action = [this]() { (*g_settings).toggleOptionAutosave(); },
			.state = [this]() { return (*g_settings).isAutosaveEnabled(); },
			.external = false,
			.divide_next = true,
		},
		{
			.title = "Otimizar Latência",
			.description = "ATIVO POR PADRÃO",
			.tooltip = "Ativo por padrão.\n\nO Jumpgate prioriza automaticamente o processo do Overwatch quando o jogo está aberto.\n\nClique para reaplicar agora. Isso pode ajudar a reduzir travamentos e oscilações de latência sem alterar bloqueios, rotas ou DNS.",
			.action = [this]() { (*g_settings).optimizeGameLatency(); },
			.external = false,
			.divide_next = true,
		},
		{
			.title = "Atualizações",
			.description = "DESATIVADAS",
			.tooltip = "Sistema preparado, mas desativado nesta build.",
			.action = []() {},
			.external = false,
			.disabled = true,
			.divide_next = true,
		},
		{
			.title = "Configurar Tunelamento",
			//.tooltip = "default: automatic\n\nThe location of Overwatch.exe must be known\nin order to enable tunneling. in most situations,\ntunneling should automatically locate your\nOverwatch.exe application. if automatically\nfinding it didn't work, a manual configuration popup will appear when you open the application. this can be ignored, but will show up again every launch unless you disable \"tunneling\" in options  • since tunneling only blocks traffic per - application, if you notice jumpgate is not working at all(you still connect to blocked servers), you may have selected an incorrect Overwatch.exe.you can always disable tunneling if you choose to. ",
			.action = [this]() {
				g_settings->setConfigTunnelingPath(std::nullopt);
				if (!g_settings->getAppSettings().options.tunneling)
				{
					(*g_settings).toggleOptionTunneling();
				}
				if (g_tunneling)
				{
					g_tunneling->requestConfiguration();
				}
			},
			.external = false,
			.divide_next = true,
		},
		{
			.title = "Configurações de Rede",
			.tooltip = "windowsdefender://network",
			.action = []() { system("start windowsdefender://network"); },
			.external = true,
		},
		{
			.title = "Regras de Firewall",
			.tooltip = "wf.msc",
			.action = []() { system("start wf.msc"); },
			.external = true,
		},
	})), i++, "Configurações"));

	this->header_actions.push_back(std::make_unique<PopupButton>("Desbloquear", "icon_clock_undo", std::move(std::vector<Action>({
		{
			.action = [this]() { (*g_settings).unblockAll(); },
		}
	})), i++, "Desbloqueia todos os servidores\n\nSe você não conseguir conectar\na um servidor, clicar aqui rapidamente\najuda a evitar uma suspensão competitiva"));

	//for (int i = 0; i < (*header_actions).size(); i++) {
	//	printf("xx %d\n", i);
	//}

}

Dashboard::~Dashboard()
{
	/*for (auto& b : (this->header_actions)) {
		delete b;
	}*/
}
