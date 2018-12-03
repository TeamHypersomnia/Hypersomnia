#include "game/modes/mode_entropy.h"

void mode_entropy::accumulate(
	const mode_player_id m_id,
	const entity_id id,
	const total_mode_player_entropy& in
) {
	cosmic[id] += in.cosmic;
	players[m_id] += in.mode;
}

total_mode_player_entropy mode_entropy::get_for(
	const mode_player_id m_id,
	const entity_id id
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

bool mode_player_entropy::is_set() const {
	return 
		team_choice != std::nullopt
		|| item_purchase != std::nullopt
	;
}

void mode_entropy::clear_dead_entities(const cosmos& cosm) {
	cosmic.clear_dead_entities(cosm);
}

void mode_entropy::clear() {
	cosmic.clear();
	players.clear();
}

bool mode_entropy::empty() const {
	return players.empty() && cosmic.empty();
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

mode_entropy& mode_entropy::operator+=(const mode_entropy& b) {
	cosmic += b.cosmic;

	for (const auto& p : b.players) {
		players[p.first] += p.second;
	}

	return *this;
}
