#pragma once
#include "application/setups/server/server_nickname_auth.h"

/*
	A tournament listed by nicknames only: its rosters hold nick_<nickname> for whoever hasn't come yet. The first
	account to come with that nickname takes its place - from then on the player is that account (as in any other
	tournament), the nickname only a label. Another account coming with it later is told it's taken.
*/

bool server_setup::claim_listed_nickname(const server_client_state& c) {
	const auto& nickname = c.get_nickname();
	const auto placeholder = ::nickname_to_account_id(nickname);
	const auto found = assigned_teams.id_to_faction.find(placeholder);

	if (nickname.empty() || found == assigned_teams.id_to_faction.end()) {
		return false;
	}

	const auto faction = found->second;
	assigned_teams.id_to_faction.erase(found);
	assigned_teams.id_to_faction[c.authenticated_id] = faction;
	claimed_nicknames.push_back(nickname);

	LOG("Tournament: %x takes the listed nickname %x, in %x.", c.authenticated_id, nickname, faction);

	if (on_listed_nickname_claimed) {
		on_listed_nickname_claimed(placeholder, c.authenticated_id);
	}

	return true;
}
