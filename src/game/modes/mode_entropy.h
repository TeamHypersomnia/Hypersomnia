#pragma once
#include "game/cosmos/cosmic_entropy.h"
#include "game/modes/mode_commands/team_choice.h"
#include "game/modes/mode_commands/item_purchase.h"
#include "augs/entity_system/storage_for_message_queues.h"
#include "game/modes/mode_player_id.h"

using mode_player_commands = augs::storage_for_message_queues<
	mode_commands::item_purchase
>;

class cosmos;

struct mode_player_entropy {
	// GEN INTROSPECTOR struct mode_player_entropy
	std::optional<mode_commands::team_choice> team_choice;
	mode_player_commands queues;
	// END GEN INTROSPECTOR
};

struct mode_entropy {
	// GEN INTROSPECTOR struct mode_entropy
	cosmic_entropy cosmic;
	std::unordered_map<mode_player_id, mode_player_entropy> players;
	// END GEN INTROSPECTOR

	void clear_dead_entities(const cosmos& cosm) {
		cosmic.clear_dead_entities(cosm);
	}

	void clear() {
		cosmic.clear();
		players.clear();
	}
};
