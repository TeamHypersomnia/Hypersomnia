#include "steam_integration.h"

#if BUILD_STEAM
#include <optional>
#include "steam_integration_callbacks.h"

const int steam_app_id = 2660970;
#include "steam_api.h"

steam_callback_queue_type steam_event_queue;
uint8_t* avatar_data = nullptr;

class new_url_launch_parameters_t {
private:
	STEAM_CALLBACK( new_url_launch_parameters_t, OnReceived, NewUrlLaunchParameters_t );
};

void new_url_launch_parameters_t::OnReceived(NewUrlLaunchParameters_t* pCallback) {
	steam_event_queue.push_back(steam_new_url_launch_parameters {});
}

std::optional<new_url_launch_parameters_t> new_url_launch_parameters_object;

template <class F>
void for_each_callback_object(F callback) {
	callback(new_url_launch_parameters_object);
}

extern "C" {
	int steam_get_appid() {
		return steam_app_id;
	}

	int steam_init() {
		if (SteamAPI_Init()) {
			for_each_callback_object([](auto& o) { o.emplace(); });

			return (int)steam_init_result::SUCCESS;
		}

		return (int)steam_init_result::FAILURE;
	}

	bool steam_restart() {
		return SteamAPI_RestartAppIfNecessary(steam_app_id);
	}

	void steam_deinit() {
		for_each_callback_object([](auto& o) { o = std::nullopt; });

		SteamAPI_Shutdown();
	}

	const char* steam_get_username() {
		return SteamFriends()->GetPersonaName();
	}

	uint8_t* steam_get_avatar(uint32_t* width, uint32_t* height) {
		int iImage = SteamFriends()->GetLargeFriendAvatar(SteamUser()->GetSteamID());

		if (iImage != 0) {
			SteamUtils()->GetImageSize(iImage, width, height);

			if (avatar_data) {
				delete [] avatar_data;
			}

			avatar_data = new uint8_t[4 * (*width) * (*height)];

			SteamUtils()->GetImageRGBA(iImage, avatar_data, 4 * (*width) * (*height));

			return avatar_data;
		}

		// Avatar retrieval failed
		*width = 0;
		*height = 0;
		return nullptr;
	}

	bool steam_set_rich_presence(const char* key, const char* value) {
		return SteamFriends()->SetRichPresence(key, value);
	}

	void steam_clear_rich_presence() {
		SteamFriends()->ClearRichPresence();
	}

	int steam_get_launch_command_line(char* buf, int bufsize) {
		return SteamApps()->GetLaunchCommandLine(buf, bufsize);
	}

	void* steam_run_callbacks() {
		steam_event_queue.clear();

		SteamAPI_RunCallbacks();

		return &steam_event_queue;
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

    uint8_t* steam_get_avatar(uint32_t* width, uint32_t* height) {
        *width = 0;
        *height = 0;
        return nullptr;
    }

	bool steam_set_rich_presence(const char*, const char*) {
		return false;
	}

	void steam_clear_rich_presence() {

	}

	int steam_get_launch_command_line(char*, int) {
		return 0;
	}

	void* steam_run_callbacks() {
		return nullptr;
	}
}

#endif
