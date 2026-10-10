#pragma once
#include <algorithm>
#include <array>
#include <string>

/*
	The authenticated account ids of the developers of the game - see get_provider_preffix -
	e.g. "steam_76561198000000000" or "discord_100000000000000000", and of any future provider.
	A server shows them with the [DEV] badge in the chat and the scoreboard,
	once it verifies their ticket - see server_setup::calc_badge.
*/

inline const auto DEVELOPER_ACCOUNT_IDS = std::array<const char*, 2> {
	"steam_76561198051900812",
	"discord_231574084931026944"
};

inline bool is_developer_account(const std::string& account_id) {
	return std::any_of(DEVELOPER_ACCOUNT_IDS.begin(), DEVELOPER_ACCOUNT_IDS.end(), [&](const char* const id) {
		return account_id == id;
	});
}
