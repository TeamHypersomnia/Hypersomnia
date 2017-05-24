#pragma once
#include <vector>
#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/transcendental/cosmic_entropy.h"
#include "game/transcendental/step_declaration.h"

struct debug_character_selection {
	std::vector<entity_id> characters;
	unsigned current_character_index = 0;
	entity_id selected_character;
	augs::window::event::state state;

	void acquire_available_characters(const cosmos& cosm) {
		int i = 0;

		while (true) {
			const auto requested_name = typesafe_sprintf(L"player%x", i);
			const auto character = cosm.get_entity_by_name(requested_name);

			if (character.dead()) {
				break;
			}

			characters.push_back(character.get_id());
			++i;
		}

		select_character(characters[0]);
	}

	void control_character_selection_numeric(
		augs::machine_entropy::local_type& changes
	) {
		using namespace augs::window::event::keys;

		erase_if(
			changes,
			[this](const auto& c) {
			state.apply(c);

			if (state.is_set(key::LCTRL) && c.was_any_key_pressed() && int(c.key.key) > int(key::_0) && int(c.key.key) <= int(key::_9)) {
				current_character_index = int(c.key.key) - int(key::_0);

				select_character(characters[current_character_index]);

				return true;
			}

			return false;
		}
		);
	}

	void control_character_selection(game_intent_vector& intents) {
		for (const auto& intent : intents) {
			if (intent.is_pressed && intent.intent == intent_type::DEBUG_SWITCH_CHARACTER) {
				++current_character_index;
				current_character_index %= characters.size();

				select_character(characters[current_character_index]);
			}
		}
	}

	entity_id get_selected_character() const {
		return selected_character;
	}

	void select_character(const entity_id h) {
		selected_character = h;
	}
};