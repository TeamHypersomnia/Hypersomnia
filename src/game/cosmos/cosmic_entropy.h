#pragma once
#include <cstddef>
#include <vector>
#include <map>

#include "augs/window_framework/event.h"

#include "game/cosmos/entity_id.h"
#include "game/enums/game_intent_type.h"
#include "game/messages/intent_message.h"
#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/detail/spells/all_spells.h"
#include "game/detail/inventory/wielding_setup.h"

#include "game/per_character_input_settings.h"

struct cosmic_entropy_recording_options;
class cosmos;

template <class key>
struct basic_player_commands {
	// GEN INTROSPECTOR struct basic_player_commands class key
	spell_id cast_spell;
	basic_wielding_setup<key> wield;
	game_intents intents;
	raw_game_motion_map motions;
	basic_item_slot_transfer_request<key> transfer;
	// END GEN INTROSPECTOR

	bool operator==(const basic_player_commands<key>& b) const;
	bool operator!=(const basic_player_commands<key>& b) const;
	basic_player_commands& operator+=(const basic_player_commands& b);

	void clear_relevant(cosmic_entropy_recording_options);
	void clear();

	std::size_t length() const;
	bool empty() const;
};

template <class key>
struct basic_player_entropy {
	// GEN INTROSPECTOR struct basic_player_entropy class key
	basic_player_commands<key> commands;
	per_character_input_settings settings;
	// END GEN INTROSPECTOR

	bool operator==(const basic_player_entropy& b) const {
		return commands == commands && settings == b.settings;
	}

	bool operator!=(const basic_player_entropy& b) const {
		return !operator==(b);
	}

	auto& operator+=(const basic_player_entropy& b) {
		commands += b.commands;
		settings = b.settings;

		return *this;
	}
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

using cosmic_player_entropy = basic_player_commands<signi_entity_id>;
using game_gui_entropy_type = cosmic_player_entropy;

struct cosmic_entropy;

struct cosmic_entropy : basic_cosmic_entropy<entity_id> {
	using base = basic_cosmic_entropy<entity_id>;
	using introspect_base = base;

	cosmic_entropy() = default;
	
	explicit cosmic_entropy(
		const per_character_input_settings& settings,
		const entity_id controlled_entity,
		const game_intents&,
		const raw_game_motion_map&
	);

	cosmic_entropy& operator+=(const cosmic_entropy& b) {
		base::operator+=(b);
		return *this;
	}
};