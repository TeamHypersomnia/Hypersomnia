#include "game/modes/mode_entropy.h"

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

mode_entropy& mode_entropy::operator+=(const mode_entropy& b) {
	cosmic += b.cosmic;

	for (const auto& p : b.players) {
		const auto& new_data = p.second;
		auto& data = players[p.first];

		if (new_data.team_choice) {
			data.team_choice = new_data.team_choice;
		}

		data.queues += new_data.queues;
	}

	return *this;
}
