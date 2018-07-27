#pragma once
#include "augs/math/declare_math.h"
#include "game/detail/inventory/requested_equipment.h"
#include "game/enums/faction_type.h"
#include "game/cosmos/solvers/standard_solver.h"
#include "game/modes/mode_entropy.h"

struct entity_guid;
struct entity_id;

class cosmos;

struct test_scene_mode_vars {
	// GEN INTROSPECTOR struct test_scene_mode_vars
	real32 respawn_after_ms = 3000;
	requested_equipment initial_eq;
	faction_type spawned_faction = faction_type::RESISTANCE;
	// END GEN INTROSPECTOR
};

class test_scene_mode {
public:
	using vars_type = test_scene_mode_vars;

	struct input {
		const test_scene_mode_vars& vars;
		cosmos& cosm;
	};

private:
	void teleport_to_next_spawn(input, entity_id character);
	void init_spawned(input, entity_id character);

	void advance_mode(input, const mode_entropy&);

public:
	// GEN INTROSPECTOR class test_scene_mode
	unsigned current_spawn_index = 0;
	// END GEN INTROSPECTOR

	entity_guid add_player(input, const faction_type);
	void remove_player(input, entity_guid);

	template <class... Callbacks>
	void advance(
		const input in, 
		const mode_entropy& entropy, 
		Callbacks&&... callbacks
	) {
		{
			const auto input = logic_step_input { in.cosm, entropy.cosmic };

			standard_solver()(
				input,
				std::forward<Callbacks>(callbacks)...
			);
		}

		advance_mode(in, entropy);
	}
};
