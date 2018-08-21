#pragma once
#include <unordered_map>

#include "augs/math/declare_math.h"
#include "game/detail/inventory/requested_equipment.h"
#include "game/enums/faction_type.h"
#include "game/cosmos/solvers/standard_solver.h"
#include "game/modes/mode_entropy.h"
#include "game/detail/economy/money_type.h"
#include "game/modes/mode_player_id.h"
#include "game/modes/mode_common.h"
#include "game/components/movement_component.h"

struct entity_guid;
struct entity_id;

class cosmos;
struct cosmos_solvable_significant;

struct bomb_mode_faction_vars {
	// GEN INTROSPECTOR struct bomb_mode_faction_vars
	requested_equipment initial_eq;
	// END GEN INTROSPECTOR
};

struct bomb_mode_vars {
	// GEN INTROSPECTOR struct bomb_mode_vars
	std::string name;
	money_type initial_money = 800;
	unsigned round_secs = 120;
	unsigned freeze_secs = 5;
	unsigned warmup_secs = 45;
	unsigned max_rounds = 5;
	unsigned warmup_respawn_after_ms = 2000;
	per_faction_t<bomb_mode_faction_vars> factions;
	// END GEN INTROSPECTOR
};

struct bomb_mode_faction_state {
	// GEN INTROSPECTOR struct bomb_mode_faction_state
	unsigned current_spawn_index = 0;
	unsigned score = 0;
	std::vector<entity_guid> shuffled_spawns;
	// END GEN INTROSPECTOR
};

struct bomb_mode_player {
	// GEN INTROSPECTOR struct bomb_mode_player
	entity_guid guid;
	entity_name_str chosen_name;
	faction_type faction = faction_type::NONE;
	money_type money = 0;
	// END GEN INTROSPECTOR

	bomb_mode_player(const entity_name_str& chosen_name = {}) : 
		chosen_name(chosen_name) 
	{}
};

enum class arena_mode_state {
	// GEN INTROSPECTOR enum class arena_mode_state
	INIT,
	WARMUP,
	LIVE
	// END GEN INTROSPECTOR
};

class bomb_mode {
public:
	using vars_type = bomb_mode_vars;
	static constexpr bool needs_initial_signi = true;
	static constexpr bool round_based = true;

	struct input {
		const bomb_mode_vars& vars;
		const cosmos_solvable_significant& initial_signi;
		cosmos& cosm;
	};

private:
	struct transferred_inventory {
		struct item {
			constrained_entity_flavour_id<components::item> flavour;

			int charges = -1;
			std::size_t container_index = static_cast<std::size_t>(-1);
			slot_function slot_type = slot_function::INVALID;
		};

		std::unordered_map<entity_id, std::size_t> id_to_container_idx;
		std::vector<item> items;
	};

	struct round_transferred_player {
		movement_flags movement;
		bool survived = false;
		transferred_inventory saved_eq;
	};

	using round_transferred_players = std::unordered_map<mode_player_id, round_transferred_player>;
	round_transferred_players make_transferred_players(input) const;

	bool still_freezed() const;
	unsigned get_round_index() const;

	void teleport_to_next_spawn(input, entity_id character);
	void init_spawned(input, entity_id character, logic_step, const round_transferred_player* = nullptr);

	void mode_pre_solve(input, const mode_entropy&, logic_step);

	void start_next_round(input, logic_step);
	void setup_round(input, logic_step, const round_transferred_players& = {});
	void reshuffle_spawns(const cosmos&, faction_type);

	bomb_mode_player* find(const mode_player_id&);
	const bomb_mode_player* find(const mode_player_id&) const;

	void set_players_frozen(input in, bool flag);
	std::size_t num_players_in_faction(faction_type) const;
	void respawn_the_dead(input, logic_step, unsigned after_ms);

public:
	// GEN INTROSPECTOR class bomb_mode
	arena_mode_state state = arena_mode_state::INIT;
	bool cache_players_frozen = false;
	per_faction_t<bomb_mode_faction_state> factions;
	std::unordered_map<mode_player_id, bomb_mode_player> players;
	// END GEN INTROSPECTOR

	mode_player_id add_player(input, const entity_name_str& chosen_name);
	void remove_player(input, const mode_player_id&);

	bool auto_assign_faction(const cosmos&, const mode_player_id&);
	bool choose_faction(const mode_player_id&, const faction_type faction);
	faction_type get_player_faction(const mode_player_id&) const;

	entity_guid lookup(const mode_player_id&) const;

	unsigned get_round_num() const;

	float get_total_seconds(input) const;

	float get_warmup_seconds_left(input) const;
	float get_match_begins_in_seconds(input) const;

	float get_freeze_seconds_left(input) const;
	float get_round_seconds_left(input) const;

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
