#pragma once
#include <unordered_map>

#include "augs/math/declare_math.h"
#include "game/modes/arena_mode.h"
#include "game/detail/inventory/requested_equipment.h"
#include "game/enums/faction_type.h"
#include "game/cosmos/solvers/standard_solver.h"
#include "game/modes/mode_entropy.h"
#include "game/detail/economy/money_type.h"
#include "game/modes/mode_player_id.h"
#include "game/modes/mode_common.h"
#include "game/components/movement_component.h"
#include "game/enums/battle_event.h"
#include "augs/misc/enum/enum_array.h"
#include "augs/misc/timing/stepped_timing.h"

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
	unsigned round_end_secs = 5;
	unsigned freeze_secs = 5;
	unsigned warmup_secs = 45;
	unsigned max_rounds = 5;
	unsigned warmup_respawn_after_ms = 2000;
	meter_value_type minimal_damage_for_assist = 41;
	per_faction_t<bomb_mode_faction_vars> factions;

	per_faction_t<per_faction_t<assets::sound_id>> win_sounds;
	per_faction_t<augs::enum_array<assets::sound_id, battle_event>> event_sounds;
	
	constrained_entity_flavour_id<invariants::explosive> bomb_flavour;
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

	int knockouts = 0;
	int assists = 0;
	int deaths = 0;
	// END GEN INTROSPECTOR

	bomb_mode_player(const entity_name_str& chosen_name = {}) : 
		chosen_name(chosen_name) 
	{}
};

enum class arena_mode_state {
	// GEN INTROSPECTOR enum class arena_mode_state
	INIT,
	WARMUP,
	LIVE,
	ROUND_END_DELAY
	// END GEN INTROSPECTOR
};

using cosmos_clock = augs::stepped_clock;

using bomb_mode_win = arena_mode_win;
using bomb_mode_knockout = arena_mode_knockout;
using bomb_mode_knockouts_vector = arena_mode_knockouts_vector;

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

	struct participating_factions {
		faction_type bombing;
		faction_type defusing;

		template <class F>
		void for_each(F callback) const {
			callback(bombing);
			callback(defusing);
		}
	};

	participating_factions calc_participating_factions(input) const;
	faction_type calc_weakest_faction(input) const;

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
	void make_win(input, faction_type);

	void teleport_to_next_spawn(input, entity_id character);
	void init_spawned(input, entity_id character, logic_step, const round_transferred_player* = nullptr);

	void mode_pre_solve(input, const mode_entropy&, logic_step);
	void mode_post_solve(input, const mode_entropy&, const_logic_step);

	void start_next_round(input, logic_step);
	void setup_round(input, logic_step, const round_transferred_players& = {});
	void reshuffle_spawns(const cosmos&, faction_type);

	bomb_mode_player* find(const mode_player_id&);
	const bomb_mode_player* find(const mode_player_id&) const;

	void set_players_frozen(input in, bool flag);
	void respawn_the_dead(input, logic_step, unsigned after_ms);

	template <class F>
	decltype(auto) on_bomb_entity(input, F) const;

	bool bomb_exploded(input) const;
	bool bomb_defused(input) const;
	bool bomb_planted(input) const;

	void play_faction_sound(const_logic_step, faction_type, assets::sound_id) const;
	void play_faction_sound_for(input, const_logic_step, battle_event, faction_type) const;

	void play_sound_for(input, const_logic_step, battle_event) const;
	void play_win_sound(input, const_logic_step, faction_type) const;

	void play_bomb_defused_sound(input, const_logic_step, faction_type) const;

	template <class F>
	void for_each_player_in(faction_type, F&& callback) const;

	template <class C, class F>
	void for_each_player_handle_in(C&, faction_type, F&& callback) const;

	std::size_t num_conscious_players_in(const cosmos&, faction_type) const;
	std::size_t num_players_in(faction_type) const;

	void process_win_conditions(input, logic_step);

	std::size_t get_round_rng_seed(const cosmos&) const;
	std::size_t get_step_rng_seed(const cosmos&) const;

	void count_knockouts_for_unconscious_players_in(input, faction_type);

	void count_knockout(input, entity_guid victim, const components::sentience&);
	void count_knockout(input, arena_mode_knockout);

public:

	// GEN INTROSPECTOR class bomb_mode
	unsigned rng_seed_offset = 0;

	cosmos_clock clock_before_setup;
	arena_mode_state state = arena_mode_state::INIT;
	bool cache_players_frozen = false;
	per_faction_t<bomb_mode_faction_state> factions;
	std::unordered_map<mode_player_id, bomb_mode_player> players;
	bomb_mode_win last_win;

	arena_mode_knockouts_vector knockouts;
	// END GEN INTROSPECTOR

	mode_player_id add_player(input, const entity_name_str& chosen_name);
	void remove_player(input, const mode_player_id&);

	bool auto_assign_faction(input, const mode_player_id&);
	bool choose_faction(const mode_player_id&, const faction_type faction);
	faction_type get_player_faction(const mode_player_id&) const;

	entity_guid lookup(const mode_player_id&) const;
	mode_player_id lookup(const entity_guid&) const;

	unsigned get_round_num() const;

	float get_total_seconds(input) const;

	float get_warmup_seconds_left(input) const;
	float get_match_begins_in_seconds(input) const;

	float get_freeze_seconds_left(input) const;
	float get_round_seconds_left(input) const;
	float get_round_end_seconds_left(input) const;

	float get_critical_seconds_left(input) const;
	float get_seconds_since_planting(input) const;

	template <class PreSolve, class PostSolve, class... Callbacks>
	void advance(
		const input in, 
		const mode_entropy& entropy, 
		PreSolve&& pre_solve,
		PostSolve&& post_solve,
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
				[&](const const_logic_step step) {
					mode_post_solve(in, entropy, step);
					post_solve(step);
				},
				std::forward<Callbacks>(callbacks)...
			);
		}
	}
};
