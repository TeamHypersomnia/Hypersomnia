#pragma once
#include "augs/math/declare_math.h"
#include "game/detail/inventory/requested_equipment.h"
#include "game/enums/faction_type.h"
#include "game/cosmos/solvers/standard_solver.h"
#include "game/modes/mode_entropy.h"
#include "game/modes/mode_player_id.h"
#include "game/modes/detail/fog_of_war_settings.h"

using mode_entity_id = entity_id;

struct entity_id;

class cosmos;

struct test_mode_view_rules {
	// GEN INTROSPECTOR struct test_mode_view_rules
	double audiovisual_speed = 1.0;
	fog_of_war_settings fog_of_war;
	bool show_enemy_hud = true;
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
	mode_entity_id controlled_character_id;
	// END GEN INTROSPECTOR

	test_mode_player(const mode_entity_id controlled_character_id = mode_entity_id()) : controlled_character_id(controlled_character_id) {}
};

class test_mode {
public:
	using ruleset_type = test_mode_ruleset;
	static constexpr bool needs_initial_signi = false;
	static constexpr bool round_based = false;

	template <bool C>
	struct basic_input {
		const ruleset_type& rules;
		maybe_const_ref_t<C, cosmos> cosm;

		template <bool is_const = C, class = std::enable_if_t<!is_const>>
		operator basic_input<!is_const>() const {
			return { rules, cosm };
		}
	};

	using input = basic_input<false>;
	using const_input = basic_input<true>;

private:
	void teleport_to_next_spawn(input, entity_id character);
	void init_spawned(input, entity_id character, logic_step);

	void mode_pre_solve(input, const mode_entropy&, logic_step);

public:
	// GEN INTROSPECTOR class test_mode
	unsigned current_spawn_index = 0;
	std::vector<mode_entity_id> pending_inits;
	std::unordered_map<mode_player_id, test_mode_player> players;
	// END GEN INTROSPECTOR

	mode_player_id add_player(input, const faction_type);
	void remove_player(input, mode_player_id);

	mode_entity_id lookup(const mode_player_id&) const;
	mode_player_id lookup(const mode_entity_id&) const;

	template <class C>
	decltype(auto) advance(
		const input in, 
		const mode_entropy& entropy, 
		C callbacks,
		const solve_settings settings
	) {
		const auto step_input = logic_step_input { in.cosm, entropy.cosmic, settings };

		return standard_solver()(
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
