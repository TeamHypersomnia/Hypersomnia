#pragma once
#include <map>

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
	std::map<mode_player_id, mode_player_entropy> players;
	// END GEN INTROSPECTOR

	void clear_dead_entities(const cosmos& cosm);
	void clear();
	bool empty() const;
	mode_entropy& operator+=(const mode_entropy& b);
};
