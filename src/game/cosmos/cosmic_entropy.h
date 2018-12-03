#pragma once
#include <vector>
#include <map>

#include "augs/window_framework/event.h"

#include "game/cosmos/entity_id.h"
#include "game/enums/game_intent_type.h"
#include "game/messages/intent_message.h"
#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/detail/spells/all_spells.h"
#include "game/detail/inventory/wielding_setup.h"

struct cosmic_entropy_recording_options;
class cosmos;

template <class key>
struct basic_player_entropy {
	// GEN INTROSPECTOR struct basic_player_entropy class key
	spell_id cast_spell;
	std::optional<basic_wielding_setup<key>> wield;
	game_intents intents;
	game_motions motions;
	std::vector<basic_item_slot_transfer_request<key>> transfers;
	// END GEN INTROSPECTOR

	bool operator==(const basic_player_entropy<key>& b) const;
	bool operator!=(const basic_player_entropy<key>& b) const;
	basic_player_entropy& operator+=(const basic_player_entropy& b);

	void clear_relevant(cosmic_entropy_recording_options);
	void clear();
};

template <class key>
struct basic_cosmic_entropy {
	using player_entropy_type = basic_player_entropy<key>;
	// GEN INTROSPECTOR struct basic_cosmic_entropy class key
	std::map<key, player_entropy_type> players;
	// END GEN INTROSPECTOR

	std::size_t length() const;

	basic_cosmic_entropy& operator+=(const basic_cosmic_entropy& b);

	void clear_dead_entities(const cosmos&);
	void clear();

	bool empty() const;

	player_entropy_type& operator[](const key& k) {
		return players[k];
	}

	const player_entropy_type& at(const key& k) const { 
		return players.at(k);
	}

	bool operator==(const basic_cosmic_entropy<key>&) const;
	bool operator!=(const basic_cosmic_entropy<key>&) const;
};

using cosmic_player_entropy = basic_player_entropy<signi_entity_id>;
using game_gui_entropy_type = cosmic_player_entropy;

struct cosmic_entropy;

struct cosmic_entropy : basic_cosmic_entropy<entity_id> {
	using base = basic_cosmic_entropy<entity_id>;
	using introspect_base = base;

	cosmic_entropy() = default;
	
	explicit cosmic_entropy(
		const entity_id controlled_entity,
		const game_intents&,
		const game_motions&
	);

	cosmic_entropy& operator+=(const cosmic_entropy& b) {
		base::operator+=(b);
		return *this;
	}
};