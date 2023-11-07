#include "steam_integration.h"

#if BUILD_STEAM

const int steam_app_id = 2660970;
#include "steam_api.h"

extern "C" {
	int steam_get_appid() {
		return steam_app_id;
	}

	int steam_init() {
		if (SteamAPI_Init()) {
			return (int)steam_init_result::SUCCESS;
		}

		return (int)steam_init_result::FAILURE;
	}

	bool steam_restart() {
		return SteamAPI_RestartAppIfNecessary(steam_app_id);
	}

	void steam_deinit() {
		SteamAPI_Shutdown();
	}

	const char* steam_get_username() {
		return SteamFriends()->GetPersonaName();
	}
}
#else
// non-steam version

extern "C" {
	int steam_get_appid() {
		return 0;
	}

	int steam_init() {
		return (int)steam_init_result::DISABLED;
	}

	bool steam_restart() {
		return false;
	}

	void steam_deinit() {

	}

	const char* steam_get_username() {
		return nullptr;
	}
}

#endif
