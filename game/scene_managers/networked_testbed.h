#pragma once
#include <vector>
#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmos.h"
#include "game/game_window.h"
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
	class networked_testbed {
		void populate(fixed_step&);

		struct controlled_character {
			controlled_character(entity_id id = entity_id()) : id(id) {}

			entity_id id;
			augs::network::endpoint_address endpoint;
			bool occupied = false;
		};

		augs::constant_size_vector<controlled_character, TESTBED_CHARACTERS_COUNT> characters;

		friend class networked_testbed_server;
	public:

		void populate_world_with_entities(cosmos&);

		void step_with_callbacks(const cosmic_entropy&, cosmos&);

		void pre_solve(fixed_step&);
		void post_solve(fixed_step&);

	};

	class networked_testbed_server : public networked_testbed {
	public:

		entity_id assign_new_character(augs::network::endpoint_address);
		void free_character(augs::network::endpoint_address);
	};

	class networked_testbed_client : public networked_testbed {
		void view_cosmos(const cosmos&, basic_viewing_step&, world_camera&) const;
	public:
		entity_id currently_controlled_character;
		bool show_profile_details = false;

		entity_id get_controlled_entity() const;
		void inject_input_to(entity_handle);
		
		cosmic_entropy make_cosmic_entropy(const augs::machine_entropy::local_type&, const input_context&, cosmos&);

		void control(const augs::machine_entropy::local_type&, cosmos&);

		void configure_view(viewing_session&) const;
		void view(const cosmos&, game_window&, viewing_session&, const augs::variable_delta& dt) const;
	};
}