#pragma once
#include <vector>
#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"

#include "augs/network/network_client.h"
#include "augs/misc/machine_entropy.h"

struct cosmic_entropy;
#include "game/transcendental/step_declaration.h"
class cosmos;
class world_camera;
class viewing_session;
struct input_context;

namespace scene_managers {
	class networked_testbed {
		void populate(const logic_step);

		struct controlled_character {
			controlled_character(entity_id id = entity_id()) : id(id) {}

			entity_id id;
			bool occupied = false;
		};

		friend class networked_testbed_server;
	public:
		augs::constant_size_vector<controlled_character, TESTBED_CHARACTERS_COUNT> characters;

		void populate_world_with_entities(cosmos&);
	};

	class networked_testbed_server : public networked_testbed {
	public:
		entity_id assign_new_character();
		void free_character(const entity_id);
	};

	class networked_testbed_client : public networked_testbed {
	public:
		entity_id selected_character;

		entity_id get_selected_character() const;
		void select_character(const entity_id);
	};
}