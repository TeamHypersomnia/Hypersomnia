#pragma once

/*
	A tournament's page shows each player's avatar. Its match servers pass on the avatars of the players who came,
	once a second with the live score: each one decoded and encoded again as PNG (avatar_png.h), only when it's new -
	a player's avatar again, unchanged, isn't sent twice.

	The player is named as the page names them: by the account (normalized - 0x... is key_...), or where the nicknames
	are the identities (nick_alice) by the nickname alone (alice).
*/

void server_setup::publish_tournament_avatars() {
	if (!on_player_avatar) {
		return;
	}

	for_each_id_and_client([&](auto, const auto& c) {
		const auto& bytes = c.meta.avatar.image_bytes;

		if (!c.is_authenticated() || bytes.empty()) {
			return;
		}

		const auto& id = c.authenticated_id;
		const auto player = ::is_valid_account_id(id) ? ::normalize_account_id(id) : ::account_id_to_nickname(id);
		const auto fingerprint = std::hash<std::string_view>()(std::string_view(reinterpret_cast<const char*>(bytes.data()), bytes.size()));

		if (const auto passed = avatars_passed_on.find(player); passed != avatars_passed_on.end() && passed->second == fingerprint) {
			return;
		}

		avatars_passed_on[player] = fingerprint;

		if (const auto png = ::reencoded_avatar_png(bytes); !png.empty()) {
			on_player_avatar(player, png);
		}
	}, only_connected_v);
}
