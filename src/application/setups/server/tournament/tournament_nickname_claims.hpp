#pragma once
#include "application/setups/server/server_nickname_auth.h"

/*
	A tournament listed by nicknames only: its rosters hold nick_<nickname> for whoever hasn't come yet. The first
	account to come with that nickname takes its place - from then on the player is that account (as in any other
	tournament), the nickname only a label. Another account coming with it later is told it's taken.

	An account already in a team never takes another one's nickname (a shared computer at a LAN): it would play for two.
	True: the claim is handled (taken, or refused and kicked).
*/

/*
	A listed nickname, whatever the case it's typed in ("Bob" takes "bob") - its placeholder as the roster has it.
*/

std::optional<std::string> server_setup::find_listed_nickname(const std::string& nickname) const {
	if (nickname.empty()) {
		return std::nullopt;
	}

	const auto wanted = ::to_lowercase(nickname);

	for (const auto& [id, faction] : assigned_teams.id_to_faction) {
		(void)faction;

		if (begins_with(id, nickname_account_id_prefix_v) && ::to_lowercase(::account_id_to_nickname(id)) == wanted) {
			return id;
		}
	}

	return std::nullopt;
}

/*
	The listed nicknames still to come on this match - named in the refusal of someone whose nickname isn't one of them.
*/

std::string server_setup::unclaimed_listed_nicknames() const {
	auto out = std::string();

	for (const auto& [id, faction] : assigned_teams.id_to_faction) {
		(void)faction;

		if (begins_with(id, nickname_account_id_prefix_v)) {
			out += (out.empty() ? "" : ", ") + ::account_id_to_nickname(id);
		}
	}

	return out;
}

bool server_setup::claim_listed_nickname(const client_id_type client_id, const server_client_state& c) {
	const auto& nickname = c.get_nickname();
	const auto listed = find_listed_nickname(nickname);

	if (!listed.has_value()) {
		return false;
	}

	const auto placeholder = *listed;
	const auto found = assigned_teams.id_to_faction.find(placeholder);

	if (on_listed_nickname_claimed && !on_listed_nickname_claimed(placeholder, c.authenticated_id)) {
		LOG("Tournament: %x may not take the listed nickname %x - taken, or the account plays under another one.", c.authenticated_id, nickname);
		kick_now(client_id, "This nickname is taken in this tournament -\nor this computer's account plays under another one.");
		return true;
	}

	const auto faction = found->second;
	assigned_teams.id_to_faction.erase(found);
	assigned_teams.id_to_faction[c.authenticated_id] = faction;
	claimed_nicknames.push_back(nickname);

	LOG("Tournament: %x takes the listed nickname %x, in %x.", c.authenticated_id, nickname, faction);

	return true;
}
