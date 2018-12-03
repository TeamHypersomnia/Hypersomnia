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

struct test_mode_view_rules {
	// GEN INTROSPECTOR struct test_mode_view_rules
	double audiovisual_speed = 1.0;
	// END GEN INTROSPECTOR
};

struct test_mode_ruleset {
	// GEN INTROSPECTOR struct test_mode_ruleset
	std::string name = "Unnamed test scene mode vars";

	real32 respawn_after_ms = 3000;
	requested_equipment initial_eq;
	faction_type spawned_faction = faction_type::RESISTANCE;
	int spawned_chars = 1;
	test_mode_view_rules view;
	// END GEN INTROSPECTOR
};

struct test_mode_player {
	// GEN INTROSPECTOR struct test_mode_player
	entity_guid guid;
	// END GEN INTROSPECTOR

	test_mode_player(const entity_guid guid = entity_guid()) : guid(guid) {}
};

class test_mode {
public:
	using ruleset_type = test_mode_ruleset;
	static constexpr bool needs_initial_signi = false;
	static constexpr bool round_based = false;

	struct input {
		const ruleset_type& rules;
		cosmos& cosm;
	};

private:
	void teleport_to_next_spawn(input, entity_id character);
	void init_spawned(input, entity_id character, logic_step);

	void mode_pre_solve(input, const mode_entropy&, logic_step);

public:
	// GEN INTROSPECTOR class test_mode
	unsigned current_spawn_index = 0;
	std::vector<entity_guid> pending_inits;
	std::unordered_map<mode_player_id, test_mode_player> players;
	// END GEN INTROSPECTOR

	mode_player_id add_player(input, const faction_type);
	void remove_player(input, mode_player_id);

	entity_guid lookup(const mode_player_id&) const;
	mode_player_id lookup(const entity_guid&) const;

	void request_restart() { players.clear(); }

	template <class C>
	void advance(
		const input in, 
		const mode_entropy& entropy, 
		C callbacks
	) {
		const auto step_input = logic_step_input { in.cosm, entropy.cosmic };

		standard_solver()(
			step_input, 
			solver_callbacks(
				[&](const logic_step step) {
					callbacks.pre_solve(step);
					mode_pre_solve(in, entropy, step);
				},
				callbacks.post_solve,
				callbacks.post_cleanup
			)
		);
	}
};
