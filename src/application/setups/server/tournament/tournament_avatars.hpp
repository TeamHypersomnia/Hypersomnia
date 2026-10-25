#pragma once
#include "application/setups/server/tournament/tournament_page_server.h"

/*
	A tournament's page shows each player's avatar. Its match servers pass on the avatars of the players who came,
	once a second with the live score: each one decoded and encoded again as PNG (avatar_png.h), only when it's new -
	a player's avatar again, unchanged, isn't sent twice.

	Only the players of the match (with a team), by their accounts.
*/

void server_setup::publish_tournament_avatars() {
	if (!on_player_avatar) {
		return;
	}

	for_each_id_and_client([&](auto, const auto& c) {
		const auto& bytes = c.meta.avatar.image_bytes;

		if (!c.is_authenticated() || bytes.empty() || !::is_actual_faction(get_assigned_team(c.authenticated_id))) {
			return;
		}

		const auto& player = c.authenticated_id;
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
