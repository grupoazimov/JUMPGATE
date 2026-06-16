#pragma once

namespace core::connection_gate
{
	enum class RemoteState
	{
		Allowed,
		NewVersionAvailable,
		VersionEnded,
		ServerBlocked,
		Unavailable,
		Invalid,
	};

	struct CheckResult
	{
		RemoteState state{ RemoteState::Unavailable };
		std::string raw;
		std::string error;
	};

	CheckResult check();
}
