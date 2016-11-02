#pragma once
#include <vector>
#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"
#include "augs/misc/machine_entropy.h"

struct cosmic_entropy;
class logic_step;
class cosmos;
class world_camera;
class viewing_session;
struct input_context;

namespace scene_managers {
	class testbed {
		void populate(logic_step&, const vec2i screen_size);
	public:
		augs::constant_size_vector<entity_id, TESTBED_CHARACTERS_COUNT> characters;
		augs::constant_size_vector<entity_id, 100> crates;
		unsigned current_character = 0;
		entity_id currently_controlled_character;
		int debug_var = 0;
		augs::constant_size_vector<entity_id, TESTBED_DRAW_BODIES_COUNT> draw_bodies;

		cosmos stashed_cosmos;
		augs::stream stashed_delta;

		void configure_view(viewing_session&) const;

		void populate_world_with_entities(cosmos&, const vec2i screen_size);

		void control(const augs::machine_entropy::local_type&, cosmos&);
		
		cosmic_entropy make_cosmic_entropy(const augs::machine_entropy::local_type&, const input_context&, const cosmos&);
		entity_id get_controlled_entity() const;

		void inject_input_to(entity_handle);

		void step_with_callbacks(const cosmic_entropy&, cosmos&, viewing_session& post_solve_effects_response);

		void pre_solve(logic_step&);
		void post_solve(logic_step&);
	};
}