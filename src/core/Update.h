#pragma once

namespace core::update
{
	struct UpdateStatus
	{
		bool enabled{ false };
		const char* label{ "Atualizações desativadas" };
	};

	UpdateStatus getStatus();
}
