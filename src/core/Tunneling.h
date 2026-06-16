#pragma once

#include "core/Settings.h"
#include "core/Firewall.h"

#include "util/win/win_filesystem/file_picker.h"

namespace core::tunneling
{

	class Tunneling
	{

		public:
			Tunneling();
			void render();
			void requestConfiguration();

		private:
			static std::set<std::string> _queryFirewallForPossibleExePaths(std::string rule_name /* = "Overwatch Application" */);

			bool _configuration_dismissed{ false };
			bool _configuration_requested{ false };
			double _configuration_opened_at{ 0.0 };
			double _path_picker_opened_at{ 0.0 };

	};

}
