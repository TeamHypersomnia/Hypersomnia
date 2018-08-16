#pragma once
#include "augs/math/declare_math.h"
#include "game/detail/inventory/requested_equipment.h"
#include "game/enums/faction_type.h"
#include "game/cosmos/solvers/standard_solver.h"
#include "game/modes/mode_entropy.h"
#include "game/modes/mode_player_id.h"

struct entity_guid;
struct entity_id;

class cosmos;

struct test_scene_mode_vars {
	// GEN INTROSPECTOR struct test_scene_mode_vars
	std::string name;

	real32 respawn_after_ms = 3000;
	requested_equipment initial_eq;
	faction_type spawned_faction = faction_type::RESISTANCE;
	// END GEN INTROSPECTOR
};

struct test_mode_player {
	// GEN INTROSPECTOR struct test_mode_player
	entity_guid guid;
	// END GEN INTROSPECTOR

	test_mode_player(const entity_guid guid = entity_guid()) : guid(guid) {}
};

class test_scene_mode {
public:
	using vars_type = test_scene_mode_vars;
	static constexpr bool needs_initial_signi = false;

	struct input {
		const test_scene_mode_vars& vars;
		cosmos& cosm;
	};

private:
	void teleport_to_next_spawn(input, entity_id character);
	void init_spawned(input, entity_id character, logic_step);

	void mode_pre_solve(input, const mode_entropy&, logic_step);

public:
	// GEN INTROSPECTOR class test_scene_mode
	unsigned current_spawn_index = 0;
	std::vector<entity_guid> pending_inits;
	std::unordered_map<mode_player_id, test_mode_player> players;
	// END GEN INTROSPECTOR

	mode_player_id add_player(input, const faction_type);
	void remove_player(input, mode_player_id);

	entity_guid lookup(const mode_player_id&) const;

	template <class PreSolve, class... Callbacks>
	void advance(
		const input in, 
		const mode_entropy& entropy, 
		PreSolve&& pre_solve,
		Callbacks&&... callbacks
	) {
		{
			const auto input = logic_step_input { in.cosm, entropy.cosmic };

			standard_solver()(
				input,
				[&](const logic_step step) {
					pre_solve(step);
					mode_pre_solve(in, entropy, step);
				},
				std::forward<Callbacks>(callbacks)...
			);
		}
	}
};
