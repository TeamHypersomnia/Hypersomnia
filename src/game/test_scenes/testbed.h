#pragma once
#include <vector>
#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"

#include "game/transcendental/cosmic_entropy.h"
#include "game/transcendental/step_declaration.h"

class cosmos;
class world_camera;
class viewing_session;

namespace scene_builders {
	class testbed {
		void populate(const logic_step);
	public:
		augs::constant_size_vector<entity_id, TESTBED_CHARACTERS_COUNT> characters;
		augs::constant_size_vector<entity_id, 100> crates;
		unsigned current_character_index = 0;
		entity_id selected_character;
		int debug_var = 0;
		augs::constant_size_vector<entity_id, TESTBED_DRAW_BODIES_COUNT> draw_bodies;

		cosmos stashed_cosmos;
		augs::stream stashed_delta;

		augs::window::event::state state;

		template <class T>
		void populate_world_with_entities(
			cosmos& cosm,
			const all_logical_metas_of_assets& metas,
			const T post_solve
		) {
			cosm.advance_deterministic_schemata(
				{ cosmic_entropy(), metas },
				[&](const logic_step step) { populate(step); }, 
				post_solve
			);
		}

		void control_character_selection_numeric(augs::machine_entropy::local_type&);
		void control_character_selection(game_intent_vector&);
		
		entity_id get_selected_character() const;

		void select_character(const entity_id);
	};
}