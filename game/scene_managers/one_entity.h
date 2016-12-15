#pragma once
#include <vector>
#include "game/transcendental/entity_id.h"

namespace augs {
	struct machine_entropy;
}

struct cosmic_entropy;
#include "game/transcendental/step_declaration.h"
class cosmos;
class viewing_session;
struct input_context;

namespace scene_managers {
	class one_entity {
		std::vector<entity_id> characters;

		unsigned current_character_index = 0;
		entity_id world_camera;

	public:
		void populate_world_with_entities(logic_step&);
		entity_id get_selected_character() const;

		void configure_view(viewing_session&) const;
	};
}