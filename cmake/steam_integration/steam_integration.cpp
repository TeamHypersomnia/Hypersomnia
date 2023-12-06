#include "steam_integration.h"

#if BUILD_STEAM
#include <optional>
#include "steam_integration_callbacks.h"

const int steam_app_id = 2660970;
#include "steam_api.h"

/*
	Test:
	steam://run/2660970//arena.hypersomnia.xyz/
*/

uint8_t* avatar_data = nullptr;

class CGameManager
{
private:
	STEAM_CALLBACK( CGameManager, OnReceivedOverlay, GameOverlayActivated_t );
	STEAM_CALLBACK( CGameManager, OnReceivedLaunchParameters, NewUrlLaunchParameters_t );
	STEAM_CALLBACK( CGameManager, OnReceivedRichPresenceJoin, GameRichPresenceJoinRequested_t );
	STEAM_CALLBACK( CGameManager, OnReceivedServerCahnge, GameServerChangeRequested_t );
public:

	steam_callback_handler_type callback_handler = nullptr;
	void* ctx = nullptr;

	template <class T>
	void handle(T event) {
		callback_handler(ctx, T::index, &event);
	}
};

void CGameManager::OnReceivedOverlay( GameOverlayActivated_t*  )
{

}

void CGameManager::OnReceivedLaunchParameters(NewUrlLaunchParameters_t* ) {
	handle(steam_new_url_launch_parameters {});
}

void CGameManager::OnReceivedRichPresenceJoin( GameRichPresenceJoinRequested_t* pCallback  )
{
	handle(steam_new_join_game_request { { std::to_array(pCallback->m_rgchConnect) } });
}

void CGameManager::OnReceivedServerCahnge( GameServerChangeRequested_t* pCallback )
{
	handle(steam_change_server_request { { std::to_array(pCallback->m_rgchServer) }, { std::to_array(pCallback->m_rgchPassword) } });
}

std::optional<CGameManager> CGameManager_object;

extern "C" {
	int steam_get_appid() {
		return steam_app_id;
	}

	int steam_init() {
		if (SteamAPI_Init()) {
			CGameManager_object.emplace();

			return (int)steam_init_result::SUCCESS;
		}

		return (int)steam_init_result::FAILURE;
	}

	bool steam_restart() {
		return SteamAPI_RestartAppIfNecessary(steam_app_id);
	}

	void steam_deinit() {
		CGameManager_object = std::nullopt;

		SteamAPI_Shutdown();
	}

	uint64_t steam_get_id() {
		return SteamUser()->GetSteamID().ConvertToUint64();
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

	void steam_run_callbacks(steam_callback_handler_type callback_handler, void* ctx) {
		CGameManager_object->callback_handler = callback_handler;
		CGameManager_object->ctx = ctx;

		SteamAPI_RunCallbacks();
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

	uint64_t steam_get_id() {
		return 0;
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

	void steam_run_callbacks(steam_callback_handler_type, void*) {

	}
}

#endif
