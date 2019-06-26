#include "augs/templates/traits/variant_traits.h"
#include "game/modes/mode_entropy.h"
#include "augs/templates/logically_empty.h"

bool total_mode_player_entropy::empty() const {
	return logically_empty(mode, cosmic);
}

total_mode_player_entropy& total_mode_player_entropy::operator+=(const total_mode_player_entropy& b) {
	if (logically_set(b.mode)) {
		mode = b.mode;
	}

	cosmic += b.cosmic;
	return *this;
}

total_mode_player_entropy mode_entropy::get_for(
	const entity_id id,
	const mode_player_id m_id
) const {
	total_mode_player_entropy out;

	if (const auto found_cosmic = mapped_or_nullptr(cosmic.players, id)) {
		out.cosmic = found_cosmic->commands;
	}

	if (const auto found_mode = mapped_or_nullptr(players, m_id)) {
		out.mode = *found_mode;
	}

	return out;
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
	added_player = {};
	removed_player = {};
	special_command = std::monostate();
}

bool mode_entropy_general::empty() const {
	return logically_empty(added_player, removed_player, special_command);
}

bool mode_entropy::empty() const {
	return logically_empty(players, cosmic, general);
}

mode_entropy_general& mode_entropy_general::operator+=(const mode_entropy_general& b) {
	auto override_if = [&](auto& a, const auto& b) {
		if (b.is_set()) {
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
		if (logically_set(p.second)) {
			players[p.first] = p.second;
		}
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

bool total_mode_player_entropy::operator==(const total_mode_player_entropy& b) const {
	return mode == b.mode && cosmic == b.cosmic;
}
