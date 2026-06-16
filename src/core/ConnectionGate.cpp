#include "pch.h"

#include "core/ConnectionGate.h"

#include <algorithm>
#include <cctype>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

namespace
{
	constexpr const char* STATUS_URL = "https://azimovesports.com/jumpgate/conexao.txt";

	std::string trim(std::string value)
	{
		value.erase(std::remove_if(value.begin(), value.end(), [](unsigned char ch) {
			return std::isspace(ch) != 0;
		}), value.end());
		return value;
	}
}

namespace core::connection_gate
{
	CheckResult check()
	{
		CheckResult result;

		HINTERNET internet = InternetOpenA(
			"AZIMOV JUMPGATE",
			INTERNET_OPEN_TYPE_PRECONFIG,
			nullptr,
			nullptr,
			0
		);

		if (!internet)
		{
			result.error = "não foi possível iniciar a conexão";
			return result;
		}

		DWORD timeout = 5000;
		InternetSetOptionA(internet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
		InternetSetOptionA(internet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
		InternetSetOptionA(internet, INTERNET_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));

		HINTERNET file = InternetOpenUrlA(
			internet,
			STATUS_URL,
			nullptr,
			0,
			INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE,
			0
		);

		if (!file)
		{
			result.error = "não foi possível consultar o status remoto";
			InternetCloseHandle(internet);
			return result;
		}

		char buffer[128]{};
		DWORD bytes_read = 0;
		if (InternetReadFile(file, buffer, sizeof(buffer) - 1, &bytes_read) && bytes_read > 0)
		{
			result.raw.assign(buffer, bytes_read);
		}
		else
		{
			result.error = "resposta remota vazia";
		}

		InternetCloseHandle(file);
		InternetCloseHandle(internet);

		const std::string value = trim(result.raw);
		if (value == "0")
		{
			result.state = RemoteState::Allowed;
		}
		else if (value == "1")
		{
			result.state = RemoteState::NewVersionAvailable;
		}
		else if (value == "2")
		{
			result.state = RemoteState::VersionEnded;
		}
		else if (value == "3")
		{
			result.state = RemoteState::ServerBlocked;
		}
		else if (!value.empty())
		{
			result.state = RemoteState::Invalid;
			result.error = "status remoto inválido: " + value;
		}

		return result;
	}
}
