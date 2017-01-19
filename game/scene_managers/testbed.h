#pragma once
#include <vector>
#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"
#include "augs/misc/machine_entropy.h"

struct cosmic_entropy;
#include "game/transcendental/step_declaration.h"
class cosmos;
class world_camera;
class viewing_session;
struct input_context;

namespace scene_managers {
	class testbed {
		void populate(const logic_step, const vec2i screen_size);
	public:
		augs::constant_size_vector<entity_id, TESTBED_CHARACTERS_COUNT> characters;
		augs::constant_size_vector<entity_id, 100> crates;
		unsigned current_character_index = 0;
		entity_id selected_character;
		int debug_var = 0;
		augs::constant_size_vector<entity_id, TESTBED_DRAW_BODIES_COUNT> draw_bodies;

		cosmos stashed_cosmos;
		augs::stream stashed_delta;

		void populate_world_with_entities(cosmos&, const vec2i screen_size);

		void control_character_selection(const augs::machine_entropy::local_type&);
		
		entity_id get_selected_character() const;

		void select_character(const entity_id);
	};
}