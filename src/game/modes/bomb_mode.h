#pragma once
#include "augs/math/declare_math.h"
#include "game/detail/inventory/requested_equipment.h"
#include "game/enums/faction_type.h"
#include "game/cosmos/solvers/standard_solver.h"
#include "game/modes/mode_entropy.h"
#include "game/detail/economy/money_type.h"
#include "augs/misc/enum/enum_array.h"
#include "game/modes/mode_player_id.h"

struct entity_guid;
struct entity_id;

class cosmos;
struct cosmos_solvable_significant;

template <class T>
using per_faction_t = augs::enum_array<T, faction_type>;

struct bomb_mode_faction_vars {
	// GEN INTROSPECTOR struct bomb_mode_faction_vars
	requested_equipment initial_eq;
	// END GEN INTROSPECTOR
};

struct bomb_mode_vars {
	// GEN INTROSPECTOR struct bomb_mode_vars
	std::string name;
	money_type initial_money = 800;
	real32 round_secs = 120.f;
	real32 freeze_secs = 5.f;
	unsigned max_rounds = 5;
	per_faction_t<bomb_mode_faction_vars> factions;
	// END GEN INTROSPECTOR
};

struct bomb_mode_faction_state {
	// GEN INTROSPECTOR struct bomb_mode_faction_state
	unsigned current_spawn_index = 0;
	unsigned score = 0;
	// END GEN INTROSPECTOR
};

struct bomb_mode_player {
	// GEN INTROSPECTOR struct bomb_mode_player
	entity_guid guid;
	entity_name_str chosen_name;
	faction_type faction = faction_type::NONE;
	// END GEN INTROSPECTOR

	bomb_mode_player(const entity_name_str& chosen_name = {}) : 
		chosen_name(chosen_name) 
	{}
};

class bomb_mode {
public:
	using vars_type = bomb_mode_vars;
	static constexpr bool needs_initial_signi = true;

	struct input {
		const bomb_mode_vars& vars;
		const cosmos_solvable_significant& initial_solvable;
		cosmos& cosm;
	};

private:
	bool still_freezed() const;
	unsigned get_round_index() const;

	void teleport_to_next_spawn(input, entity_id character);
	void init_spawned(input, entity_id character, logic_step);

	void mode_pre_solve(input, const mode_entropy&, logic_step);

public:
	// GEN INTROSPECTOR class bomb_mode
	per_faction_t<bomb_mode_faction_state> factions;
	std::vector<entity_guid> pending_inits;
	std::unordered_map<mode_player_id, bomb_mode_player> players;
	// END GEN INTROSPECTOR

	mode_player_id add_player(input, const entity_name_str& chosen_name);
	void remove_player(input, const mode_player_id&);

	bool choose_faction(const mode_player_id&, const faction_type faction);
	faction_type get_player_faction(const mode_player_id&) const;

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
