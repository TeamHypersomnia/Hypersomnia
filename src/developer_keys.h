#pragma once
#include <algorithm>
#include <array>
#include <cctype>
#include <string>

/*
	The account ids of the developers of the game - see identity_cert.h - e.g. "steam_76561198000000000", and their
	account keys ("key_" and the public key in hex), as which a key linked to Steam counts outside the Steam build.
	A server shows them with the [DEV] badge in the chat and the scoreboard, once their cert is verified -
	see server_setup::calc_badge.
*/

inline const auto DEVELOPER_ACCOUNT_IDS = std::array<const char*, 2> {
	"steam_76561198051900812",
	"key_b0bde74674c1317ab120d58680f7c6e6fcc8e92f55fc7881ba7f3eb2a7c57631"
};

inline bool is_developer_badge(std::string badge) {
	std::transform(badge.begin(), badge.end(), badge.begin(), [](const unsigned char c) {
		return static_cast<char>(std::toupper(c));
	});

	return badge == "DEV";
}

inline bool is_developer_account(const std::string& account_id) {
	return std::any_of(DEVELOPER_ACCOUNT_IDS.begin(), DEVELOPER_ACCOUNT_IDS.end(), [&](const char* const id) {
		return account_id == id;
	});
}
