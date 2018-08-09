#pragma once
#include "augs/math/declare_math.h"
#include "game/detail/inventory/requested_equipment.h"
#include "game/enums/faction_type.h"
#include "game/cosmos/solvers/standard_solver.h"
#include "game/modes/mode_entropy.h"
#include "game/detail/economy/money_type.h"
#include "augs/misc/enum/enum_array.h"

struct entity_guid;
struct entity_id;

class cosmos;
class cosmos_solvable;

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

class bomb_mode {
public:
	using vars_type = bomb_mode_vars;
	static constexpr bool needs_initial_solvable = true;

	struct input {
		const bomb_mode_vars& vars;
		const cosmos_solvable& initial_solvable;
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
	// END GEN INTROSPECTOR

	void add_player(input, const faction_type);
	void remove_player(input, entity_guid);

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
