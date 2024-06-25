#pragma once

enum class work_result {
	STEAM_RESTART,

	SUCCESS,
	FAILURE,

	RELAUNCH_DEDICATED_SERVER,
	RELAUNCH_MASTERSERVER,
	RELAUNCH_AND_UPDATE_DEDICATED_SERVER,
	RELAUNCH_UPGRADED,

	REPORT_UPDATE_AVAILABLE,
	REPORT_UPDATE_UNAVAILABLE
};

inline auto describe_work_result(const work_result w) {
	using W = work_result;

	switch (w) {
		case W::STEAM_RESTART:
			return "Game was started without Steam client. Restarting.";

		case W::SUCCESS:
			return "Success.";
		case W::FAILURE:
			return "Failure.";

		case W::RELAUNCH_DEDICATED_SERVER:
			return "main: Dedicated server requested relaunch.";
		case W::RELAUNCH_MASTERSERVER:
			return "main: Masterserver requested relaunch.";
		case W::RELAUNCH_AND_UPDATE_DEDICATED_SERVER:
			return "main: Dedicated server detected available updates.";
		case W::RELAUNCH_UPGRADED:
			return "main: Application requested relaunch due to a successful upgrade.";

		case W::REPORT_UPDATE_AVAILABLE:
			return "Update available.";
		case W::REPORT_UPDATE_UNAVAILABLE:
			return "Update unavailable.";

		default:
			return "Unknown work result.";
	}
}
