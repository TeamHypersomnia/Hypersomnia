#pragma once
#include <vector>
#include "game/transcendental/entity_id.h"

namespace augs {
	struct machine_entropy;
}

struct cosmic_entropy;
class basic_viewing_step;
class fixed_step;
class cosmos;

namespace scene_managers {
	class testbed {
		std::vector<entity_id> characters;
		
		unsigned current_character = 0;
		entity_id world_camera;

		std::vector<entity_id> draw_bodies;

	public:
		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(characters),

				CEREAL_NVP(current_character),
				CEREAL_NVP(world_camera),

				CEREAL_NVP(draw_bodies)
			);
		}

		void populate_world_with_entities(fixed_step&);
		cosmic_entropy make_cosmic_entropy(augs::machine_entropy, cosmos&);
		entity_id get_controlled_entity() const;

		void pre_solve(fixed_step&);
		void post_solve(fixed_step&);
		
		void view_cosmos(basic_viewing_step&) const;
	};
}