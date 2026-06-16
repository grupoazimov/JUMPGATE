#include "pch.h"

#include "Firewall.h"

Firewall::Firewall()
{
	// Initialize COM.
	this->_coInitilizeSuccess = SUCCEEDED(CoInitialize(0));

	if (!(this->_coInitilizeSuccess)) {
		throw std::runtime_error("Firewall do Windows: falha ao inicializar COM");
	}

	// TODO ensure rule exists
	this->_validateRules();

	this->_queryNetworkStatus();

	// TODO legacy


	/*
	
	legacy: always remove stormy.gg/jumpgate
	new group name: stormy/jumpgate

	legacy: always remove stormy.gg/jumpgate
	new group name: stormy/jumpgate

	legacy: always remove stormy.gg/jumpgate
	new group name: stormy/jumpgate

	legacy: always remove stormy.gg/jumpgate
	new group name: stormy/jumpgate
	
	*/

}

bool Firewall::isNetworkProtected() const
{
	return this->_network_information.connected_networks_are_enabled_in_firewall;
}

int Firewall::getConnectedNetworksCount() const
{
	return this->_network_information.connected_networks_count;
}

Firewall::~Firewall() {
	// Uninitialize COM.
	if (this->_coInitilizeSuccess) {
		CoUninitialize();
	}
}

void Firewall::_queryNetworkStatus() {

#ifdef _DEBUG
	//util::timer::Timer timer ("_queryNetworkStatus");
#endif

	//this->_network_information = std::make_optional<util::win_network::NetworkInformation>(util::win_network::queryNetwork());
	this->_network_information = util::win_network::queryNetwork();

}

void Firewall::tryWriteSettingsToFirewall(std::string data, std::string block, std::optional<std::filesystem::path> tunneling_path) {

	println("armazenamento: {} / 1024", data.length());

	util::win_firewall::forFirewallRulesInGroup(this->__group_name, [&data, &block, &tunneling_path](const CComPtr<INetFwRule>& FwRule, const CComPtr<INetFwRules>& rules) {

		CComBSTR description (data.c_str());
		if (SUCCEEDED(FwRule->put_Description(description)))
		{
			printf("descrição gravada com sucesso\n");
		}

		if (tunneling_path)
		{
			CComBSTR application_name (tunneling_path.value().c_str());
			if (SUCCEEDED(FwRule->put_ApplicationName(application_name)))
			{
				printf("nome do aplicativo gravado com sucesso\n");
			}
		}
		else
		{
			if (SUCCEEDED(FwRule->put_ApplicationName(NULL)))
			{
				printf("nome do aplicativo removido com sucesso\n");
			}
		}


		/*
			. if no blocks, unblock and set scope to ""
			. if blocks, concat and block

		*/

		if (block.empty()) {
			println("nenhum bloqueio ativo, desbloqueando...");

			if (FAILED(FwRule->put_Enabled(VARIANT_FALSE)))
			{
				printf("falha ao desbloquear\n");
			}
		}
		else {

			CComBSTR blocked_addresses (block.c_str());

			if (FAILED(FwRule->put_RemoteAddresses(blocked_addresses)))
			{
				printf("falha ao bloquear endereços\n");
			}
			else {
				/* !important make sure remote addresses are not blank. */
				/* !important otherwise this blocks all internet traffic permanently */
				if (FAILED(FwRule->put_Enabled(VARIANT_TRUE)))
				{
					printf("falha ao bloquear\n");
				}
			}
		}


	});
}

std::optional<std::string> Firewall::tryFetchSettingsFromFirewall() {

	std::optional<std::string> loaded_settings = std::nullopt;

	util::win_firewall::forFirewallRulesInGroup(this->__group_name, [&loaded_settings](const CComPtr<INetFwRule>& FwRule, const CComPtr<INetFwRules>& rules) {

		//USES_CONVERSION;
		CComBSTR description;
		if (SUCCEEDED(FwRule->get_Description(&description)) && description)
		{

			/*const auto ws = std::wstring(description, SysStringLen(description));
			std::string s(ws.begin(), ws.end());*/

			const BSTR raw_description = description;
			const int length = WideCharToMultiByte(CP_UTF8, 0, raw_description, SysStringLen(raw_description), nullptr, 0, nullptr, nullptr);
			std::string s(static_cast<size_t>(length), '\0');
			WideCharToMultiByte(CP_UTF8, 0, raw_description, SysStringLen(raw_description), s.data(), length, nullptr, nullptr);

			loaded_settings = std::make_optional<std::string>(s);

			println("caracteres do patch: {}/1024", SysStringByteLen(raw_description));
		}	
	});


	return loaded_settings;
}


/*
	.. currently, ensure a single rule exists
	.. in future, may want to ensure a single out and single in rule exist
*/
void Firewall::_validateRules() {

#ifdef _DEBUG
	util::timer::Timer timer("_validateRules");
#endif

	int c = 0;

	/* legacy */
	{	
		/* delete all stormy.gg/jumpgate blocks */
		util::win_firewall::forFirewallRulesInGroup(this->__group_name_legacy, [&c](const CComPtr<INetFwRule>& FwRule, const CComPtr<INetFwRules>& FwRules) {
			CComBSTR ruleName;
			FwRule->get_Name(&ruleName);
			FwRules->Remove(ruleName);
		});
	}

	util::win_firewall::forFirewallRulesInGroup(this->__group_name, [&c](const CComPtr<INetFwRule>& FwRule, const CComPtr<INetFwRules>& rules) {
		c ++;
	});

	if (c != 1) {
		// delete all rules
		util::win_firewall::forFirewallRulesInGroup(this->__group_name, [&c](const CComPtr<INetFwRule>& FwRule, const CComPtr<INetFwRules>& FwRules) {
			CComBSTR ruleName;
			if (FAILED(FwRule->get_Name(&ruleName)) && ruleName)
			{
				printf("falha ao obter o nome da regra\n");
			}
			if (FAILED(FwRules->Remove(ruleName)))
			{
				printf("falha ao excluir a regra\n");
			};
		});


		// add single rule
		util::win_firewall::firewallRulesPredicate([this](const CComPtr<INetFwRules>& FwRules)
		{
			CComBSTR rule_name ("stormy/jumpgate");
			CComBSTR group_name (this->__group_name.c_str());
			//CComBSTR remote_addresses ("");
			NET_FW_RULE_DIRECTION_ dir = NET_FW_RULE_DIR_OUT;
			NET_FW_PROFILE_TYPE2_ profile = NET_FW_PROFILE2_ALL;

			CComPtr<INetFwRule> pFwRule;
			if (FAILED(CoCreateInstance(__uuidof(NetFwRule), nullptr, CLSCTX_INPROC_SERVER, __uuidof(INetFwRule), (void**)&pFwRule)))
			{
				printf("falha ao criar instância da regra de firewall\n");
			}

			// Populate the Firewall Rule object
			pFwRule->put_Name(rule_name);
			//pFwRule->put_Description(bstrRuleDescription);




			// TODO utfdecode
			//pFwRule->put_ApplicationName(bstrRuleApplication);




			pFwRule->put_Protocol(NET_FW_IP_PROTOCOL_ANY);
			//pFwRule->put_RemoteAddresses(remote_addresses);
			pFwRule->put_Direction(dir);
			pFwRule->put_Grouping(group_name);
			pFwRule->put_Profiles(profile);
			pFwRule->put_Action(NET_FW_ACTION_BLOCK);
			pFwRule->put_Enabled(VARIANT_FALSE);

			// Add the Firewall Rule
			if (FAILED(FwRules->Add(pFwRule)))
			{
				printf("falha ao adicionar regra de firewall: \n");
			}

			else printf("regra de firewall adicionada\n");
		});
	}
}
