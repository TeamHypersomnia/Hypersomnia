#pragma once
#include <vector>
#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmos.h"
#include "application/game_window.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"

#include "augs/network/network_client.h"

namespace augs {
	struct machine_entropy;
}

struct cosmic_entropy;
class fixed_step;
class cosmos;
class world_camera;
class viewing_session;
struct input_context;

namespace scene_managers {
	class networked_testbed {
		void populate(fixed_step&);

		struct controlled_character {
			controlled_character(entity_id id = entity_id()) : id(id) {}

			entity_id id;
			bool occupied = false;
		};

		friend class networked_testbed_server;
	public:
		augs::constant_size_vector<controlled_character, TESTBED_CHARACTERS_COUNT> characters;

		void populate_world_with_entities(cosmos&);

		void step_with_callbacks(const cosmic_entropy&, cosmos&);

		void pre_solve(fixed_step&);
		void post_solve(fixed_step&);

	};

	class networked_testbed_server : public networked_testbed {
	public:

		entity_id assign_new_character();
		void free_character(const entity_id);
	};

	class networked_testbed_client : public networked_testbed {
	public:
		entity_id currently_controlled_character;
		bool show_profile_details = false;

		entity_id get_controlled_entity() const;
		void inject_input_to(entity_handle);
		
		cosmic_entropy make_cosmic_entropy(const augs::machine_entropy::local_type&, const input_context&, const cosmos&);

		void control(const augs::machine_entropy::local_type&, cosmos&);

		void configure_view(viewing_session&) const;
	};
}