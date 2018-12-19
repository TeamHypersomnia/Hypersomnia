#include "augs/templates/traits/variant_traits.h"
#include "game/modes/mode_entropy.h"
#include "augs/templates/logically_empty.h"

bool total_mode_player_entropy::empty() const {
	return mode.empty() && cosmic.empty();
}

total_mode_player_entropy& total_mode_player_entropy::operator+=(const total_mode_player_entropy& b) {
	mode += b.mode;
	cosmic += b.cosmic;
	return *this;
}

void mode_entropy::accumulate(
	const mode_player_id m_id,
	const entity_id id,
	const total_mode_player_entropy& in
) {
	if (id.is_set()) {
		cosmic[id] += in.cosmic;
	}

	if (m_id.is_set()) {
		players[m_id] += in.mode;
	}
}

total_mode_player_entropy mode_entropy::get_for(
	const entity_id id,
	const mode_player_id m_id
) const {
	total_mode_player_entropy out;

	if (const auto found_cosmic = mapped_or_nullptr(cosmic.players, id)) {
		out.cosmic = *found_cosmic;
	}

	if (const auto found_mode = mapped_or_nullptr(players, m_id)) {
		out.mode = *found_mode;
	}

	return out;
}

void mode_player_entropy::clear() {
	/* This is cheap enough. */
	*this = {};
}

bool mode_player_entropy::empty() const {
	return logically_empty(team_choice, item_purchase);
}

void mode_entropy::clear_dead_entities(const cosmos& cosm) {
	cosmic.clear_dead_entities(cosm);
}

void mode_entropy::clear() {
	cosmic.clear();
	players.clear();
	general.clear();
}

void mode_entropy_general::clear() {
	added_player.reset();
	removed_player.reset();
	special_command = std::monostate();
}

bool mode_entropy_general::empty() const {
	return logically_empty(added_player, removed_player, special_command);
}

bool mode_entropy::empty() const {
	return logically_empty(players, cosmic, general);
}

mode_player_entropy& mode_player_entropy::operator+=(const mode_player_entropy& b) {
	if (b.team_choice) {
		team_choice = b.team_choice;
	}

	if (b.item_purchase) {
		item_purchase = b.item_purchase;
	}

#if MODE_ENTROPY_HAS_QUEUES
	queues += b.queues;
#endif

	return *this;
}

mode_entropy_general& mode_entropy_general::operator+=(const mode_entropy_general& b) {
	auto override_if = [&](auto& a, const auto& b) {
		if (b != std::nullopt) {
			a = b;
		}
	};

	override_if(added_player, b.added_player);
	override_if(removed_player, b.removed_player);

	if (!holds_monostate(b.special_command)) {
		special_command = b.special_command;
	}

	return *this;
}

mode_entropy& mode_entropy::operator+=(const mode_entropy& b) {
	cosmic += b.cosmic;

	for (const auto& p : b.players) {
		players[p.first] += p.second;
	}

	general += b.general;

	return *this;
}

bool mode_entropy::operator==(const mode_entropy& b) const {
	return 
		general == b.general
		&& players == b.players
		&& cosmic == b.cosmic
	;
}

bool mode_entropy_general::operator==(const mode_entropy_general& b) const {
	return 
		added_player == b.added_player
		&& removed_player == b.removed_player
		&& special_command == b.special_command
	;
}

bool mode_player_entropy::operator==(const mode_player_entropy& b) const {
	return 
		team_choice == b.team_choice
		&& item_purchase == b.item_purchase
	;
}

bool total_mode_player_entropy::operator==(const total_mode_player_entropy& b) const {
	return mode == b.mode && cosmic == b.cosmic;
}
