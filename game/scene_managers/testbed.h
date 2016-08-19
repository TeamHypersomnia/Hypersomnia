#pragma once
#include <vector>
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"

namespace augs {
	struct machine_entropy;
}

struct cosmic_entropy;
class basic_viewing_step;
class fixed_step;
class cosmos;
class world_camera;
class viewing_session;
struct input_context;

namespace scene_managers {
	class testbed {
	public:
		augs::constant_size_vector<entity_id, TESTBED_CHARACTERS_COUNT> characters;
		unsigned current_character = 0;
		entity_id currently_controlled_character;
		augs::constant_size_vector<entity_id, TESTBED_DRAW_BODIES_COUNT> draw_bodies;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(characters),
				CEREAL_NVP(current_character),
				CEREAL_NVP(currently_controlled_character),
				CEREAL_NVP(draw_bodies)
			);
		}

		void configure_view(viewing_session&) const;

		void populate_world_with_entities(fixed_step&);
		cosmic_entropy make_cosmic_entropy(const augs::machine_entropy&, const input_context&, cosmos&);
		entity_id get_controlled_entity() const;

		void inject_input_to(entity_handle);

		void pre_solve(fixed_step&);
		void post_solve(fixed_step&);
		
		void view_cosmos(basic_viewing_step&, world_camera&) const;
	};
}