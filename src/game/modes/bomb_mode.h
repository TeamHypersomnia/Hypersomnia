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
#include "augs/misc/timing/speed_vars.h"
#include "game/modes/mode_commands/mode_entropy_structs.h"
#include "game/detail/view_input/predictability_info.h"

class cosmos;
struct cosmos_solvable_significant;

struct bomb_mode_faction_rules {
	// GEN INTROSPECTOR struct bomb_mode_faction_rules
	requested_equipment initial_eq;
	requested_equipment warmup_initial_eq;
	// END GEN INTROSPECTOR
};

struct bomb_mode_economy_rules {
	// GEN INTROSPECTOR struct bomb_mode_economy_rules
	money_type initial_money = 2000;
	money_type maximum_money = 16000;
	money_type warmup_initial_money = 16000;

	money_type losing_faction_award = 1500;
	money_type winning_faction_award = 3250;
	money_type consecutive_loss_bonus = 500;
	unsigned max_consecutive_loss_bonuses = 4;

	money_type team_kill_penalty = 500;

	money_type lost_but_bomb_planted_team_bonus = 500;
	money_type defused_team_bonus = 800;

	money_type bomb_plant_award = 250;
	money_type bomb_explosion_award = 350;
	money_type bomb_defuse_award = 500;

	bool give_extra_mags_on_first_purchase = true;
	// END GEN INTROSPECTOR
};

struct bomb_mode_view_rules : arena_mode_view_rules {
	using base = arena_mode_view_rules;
	using introspect_base = base;
	using theme_flavour_type = base::theme_flavour_type;

	// GEN INTROSPECTOR struct bomb_mode_view_rules
	theme_flavour_type bomb_soon_explodes_theme;
	unsigned secs_until_detonation_to_start_theme = 10;
	// END GEN INTROSPECTOR
};

struct bomb_mode_ruleset {
	// GEN INTROSPECTOR struct bomb_mode_ruleset
	std::string name = "Unnamed bomb mode ruleset";

	std::vector<entity_name_str> bot_names;
	unsigned bot_quota = 8;

	unsigned allow_spawn_for_secs_after_starting = 10;
	unsigned max_players_per_team = 32;
	unsigned round_secs = 120;
	unsigned round_end_secs = 5;
	unsigned freeze_secs = 10;
	unsigned buy_secs_after_freeze = 10;
	unsigned warmup_secs = 45;
	unsigned warmup_respawn_after_ms = 2000;
	unsigned max_rounds = 30;
	unsigned match_summary_seconds = 15;
	unsigned game_commencing_seconds = 3;
	meter_value_type minimal_damage_for_assist = 41;
	per_faction_t<bomb_mode_faction_rules> factions;

	constrained_entity_flavour_id<invariants::explosive, invariants::hand_fuse> bomb_flavour;
	bool delete_lying_items_on_round_start = false;
	bool delete_lying_items_on_warmup = true;
	bool allow_game_commencing = true;

	bomb_mode_economy_rules economy;
	bomb_mode_view_rules view;

	augs::speed_vars speeds;
	// END GEN INTROSPECTOR

	auto get_num_rounds() const {
		/* Make it even */
		return std::max((max_rounds / 2) * 2, 2u);
	}
};

struct bomb_mode_faction_state {
	// GEN INTROSPECTOR struct bomb_mode_faction_state
	unsigned current_spawn_index = 0;
	unsigned score = 0;
	unsigned consecutive_losses = 0;
	std::vector<mode_entity_id> shuffled_spawns;
	// END GEN INTROSPECTOR

	void clear_for_next_half() {
		current_spawn_index = 0;
		consecutive_losses = 0;
		shuffled_spawns.clear();
	}
};

struct arena_mode_player_round_state {
	// GEN INTROSPECTOR struct arena_mode_player_round_state
	item_purchases_vector done_purchases;
	arena_mode_awards_vector awards;
	// END GEN INTROSPECTOR
};

struct bomb_mode_player_stats {
	// GEN INTROSPECTOR struct bomb_mode_player_stats
	money_type money = 0;

	int knockouts = 0;
	int assists = 0;
	int deaths = 0;

	int bomb_plants = 0;
	int bomb_explosions = 0;

	int bomb_defuses = 0;

	arena_mode_player_round_state round_state;
	// END GEN INTROSPECTOR

	int calc_score() const;
};

struct bomb_mode_player {
	// GEN INTROSPECTOR struct bomb_mode_player
	entity_id controlled_character_id;
	entity_name_str chosen_name;
	faction_type faction = faction_type::SPECTATOR;
	bomb_mode_player_stats stats;
	uint32_t round_when_chosen_faction = static_cast<uint32_t>(-1); 
	bool is_bot = false;
	// END GEN INTROSPECTOR

	bomb_mode_player(const entity_name_str& chosen_name = {}) : 
		chosen_name(chosen_name) 
	{}

	bool operator<(const bomb_mode_player& b) const;

	bool is_set() const;
};

enum class arena_mode_state {
	// GEN INTROSPECTOR enum class arena_mode_state
	INIT,
	WARMUP,
	LIVE,
	ROUND_END_DELAY,
	MATCH_SUMMARY
	// END GEN INTROSPECTOR
};

using cosmos_clock = augs::stepped_clock;

using bomb_mode_win = arena_mode_win;
using bomb_mode_knockout = arena_mode_knockout;
using bomb_mode_knockouts_vector = arena_mode_knockouts_vector;

struct bomb_mode_round_state {
	// GEN INTROSPECTOR struct bomb_mode_round_state
	bool cache_players_frozen = false;
	bomb_mode_win last_win;
	arena_mode_knockouts_vector knockouts;
	mode_player_id bomb_planter;
	// END GEN INTROSPECTOR
};

enum class faction_choice_result {
	// GEN INTROSPECTOR enum class faction_choice_result
	FAILED,
	THE_SAME,
	CHOOSING_TOO_FAST,
	BEST_BALANCE_ALREADY,
	TEAM_IS_FULL,
	GAME_IS_FULL,
	CHANGED
	// END GEN INTROSPECTOR
};

enum class round_start_type {
	KEEP_EQUIPMENTS,
	DONT_KEEP_EQUIPMENTS
};

class bomb_mode {
public:
	using ruleset_type = bomb_mode_ruleset;
	static constexpr bool needs_initial_signi = true;
	static constexpr bool round_based = true;

	template <bool C>
	struct basic_input {
		const ruleset_type& rules;
		const cosmos_solvable_significant& initial_signi;
		maybe_const_ref_t<C, cosmos> cosm;

		template <bool is_const = C, class = std::enable_if_t<!is_const>>
		operator basic_input<!is_const>() const {
			return { rules, initial_signi, cosm };
		}
	};

	using input = basic_input<false>;
	using const_input = basic_input<true>;

	struct participating_factions {
		faction_type bombing = faction_type::SPECTATOR;
		faction_type defusing = faction_type::SPECTATOR;

		template <class F>
		void for_each(F callback) const {
			callback(bombing);
			callback(defusing);
		}

		std::size_t size() const {
			return 2;
		}

		void make_swapped(faction_type& f) const {
			if (f == bombing) {
				f = defusing;
				return;
			}

			if (f == defusing) {
				f = bombing;
				return;
			}
		}
	};

	participating_factions calc_participating_factions(const_input) const;
	faction_type calc_weakest_faction(const_input) const;

	bool is_halfway_round(const_input) const;
	bool is_final_round(const_input) const;

	bomb_mode_player_stats* stats_of(const mode_player_id&);

private:
	struct transferred_inventory {
		struct item {
			constrained_entity_flavour_id<components::item> flavour;

			int charges = -1;
			std::size_t container_index = static_cast<std::size_t>(-1);
			slot_function slot_type = slot_function::INVALID;
			entity_id source_entity_id;
		};

		std::unordered_map<entity_id, std::size_t> id_to_container_idx;
		std::vector<item> items;
	};

	struct round_transferred_player {
		movement_flags movement;
		bool survived = false;
		transferred_inventory saved_eq;
		learnt_spells_array_type saved_spells;
	};

	struct transfer_meta {
		const round_transferred_player& player;
		messages::changed_identities_message& msg;
	};

	using round_transferred_players = std::unordered_map<mode_player_id, round_transferred_player>;
	round_transferred_players make_transferred_players(input) const;

	bool still_freezed() const;
	unsigned get_round_index() const;
	void make_win(input, faction_type);

	entity_id create_character_for_player(input, logic_step, mode_player_id, std::optional<transfer_meta> = std::nullopt);

	void teleport_to_next_spawn(input, entity_id character);
	void init_spawned(
		input, 
		entity_id character, 
		logic_step, 
		std::optional<transfer_meta> = std::nullopt
	);

	void mode_pre_solve(input, const mode_entropy&, logic_step);
	void mode_post_solve(input, const mode_entropy&, const_logic_step);

	void start_next_round(input, logic_step, round_start_type = round_start_type::KEEP_EQUIPMENTS);
	void setup_round(input, logic_step, const round_transferred_players& = {});
	void reshuffle_spawns(const cosmos&, faction_type);

	void set_players_frozen(input in, bool flag);
	void release_triggers_of_weapons_of_players(input in);

	void respawn_the_dead(input, logic_step, unsigned after_ms);

	template <class F>
	decltype(auto) on_bomb_entity(const_input, F) const;

	bool bomb_exploded(const const_input) const;
	entity_id get_character_who_defused_bomb(const_input) const;
	bool bomb_planted(const_input) const;

	void play_faction_sound(const_logic_step, faction_type, assets::sound_id, predictability_info) const;
	void play_faction_sound_for(input, const_logic_step, battle_event, faction_type, predictability_info) const;

	void play_sound_for(input, const_logic_step, battle_event, predictability_info) const;
	void play_win_sound(input, const_logic_step, faction_type) const;
	void play_win_theme(input, const_logic_step, faction_type) const;

	void play_bomb_defused_sound(input, const_logic_step, faction_type) const;

	void process_win_conditions(input, logic_step);

	std::size_t get_round_rng_seed(const cosmos&) const;
	std::size_t get_step_rng_seed(const cosmos&) const;

	void count_knockouts_for_unconscious_players_in(input, faction_type);

	void count_knockout(input, entity_id victim, const components::sentience&);
	void count_knockout(input, arena_mode_knockout);

	entity_handle spawn_bomb(input);
	bool give_bomb_to_random_player(input, logic_step);
	void spawn_bomb_near_players(input);

	void execute_player_commands(input, mode_entropy&, logic_step);
	void add_or_remove_players(input, const mode_entropy&, logic_step);
	void handle_special_commands(input, const mode_entropy&, logic_step);
	void spawn_characters_for_recently_assigned(input, logic_step);
	void spawn_and_kick_bots(input, logic_step);

	void handle_game_commencing(input, logic_step);

	template <class S>
	static auto find_player_by_impl(S& self, const entity_name_str& chosen_name);

public:

	// GEN INTROSPECTOR class bomb_mode
	unsigned rng_seed_offset = 0;

	cosmos_clock clock_before_setup;
	arena_mode_state state = arena_mode_state::INIT;
	per_faction_t<bomb_mode_faction_state> factions;
	std::map<mode_player_id, bomb_mode_player> players;
	bomb_mode_round_state current_round;

	bool should_commence_when_ready = false;
	real32 commencing_timer_ms = -1.f;
	unsigned current_num_bots = 0;
	augs::speed_vars round_speeds;
	entity_id bomb_detonation_theme;
	// END GEN INTROSPECTOR

	mode_player_id add_player(input, const entity_name_str& chosen_name);

	/* A server might provide its own integer-based identifiers */
	bool add_player_custom(input, const add_player_input&);

	void remove_player(input, logic_step, const mode_player_id&);

	faction_choice_result auto_assign_faction(input, const mode_player_id&);
	faction_choice_result choose_faction(const mode_player_id&, const faction_type faction);
	faction_type get_player_faction(const mode_player_id&) const;

	mode_entity_id lookup(const mode_player_id&) const;
	mode_player_id lookup(const mode_entity_id&) const;

	unsigned get_round_num() const;

	float get_total_seconds(const_input) const;

	float get_warmup_seconds_left(const_input) const;
	float get_match_begins_in_seconds(const_input) const;

	float get_freeze_seconds_left(const_input) const;
	float get_round_seconds_passed(const_input) const;
	float get_round_seconds_left(const_input) const;
	float get_seconds_since_win(const_input) const;
	float get_match_summary_seconds_left(const_input) const;
	float get_round_end_seconds_left(const_input) const;
	float get_buy_seconds_left(const_input) const;

	real32 get_critical_seconds_left(const_input) const;
	float get_seconds_since_planting(const_input) const;

	unsigned calc_max_faction_score() const;

	std::size_t num_conscious_players_in(const cosmos&, faction_type) const;
	std::size_t num_players_in(faction_type) const;

	mode_player_id find_first_free_player() const;

	std::optional<arena_mode_match_result> calc_match_result(const_input) const;

	unsigned get_score(faction_type) const;

	void restart(input, logic_step);

	void reset_players_stats(input);
	void set_players_money_to_initial(input);
	void clear_players_round_state(input);

	void post_award(input, mode_player_id, money_type amount);

	bomb_mode_player* find_player_by(const entity_name_str& chosen_name);
	const bomb_mode_player* find_player_by(const entity_name_str& chosen_name) const;

	bomb_mode_player* find(const mode_player_id&);
	const bomb_mode_player* find(const mode_player_id&) const;

	template <class C, class F>
	decltype(auto) on_player_handle(C&, const mode_player_id&, F&& callback) const;

	template <class F>
	void for_each_player_in(faction_type, F&& callback) const;

	template <class C, class F>
	void for_each_player_handle_in(C&, faction_type, F&& callback) const;

	template <class C>
	decltype(auto) advance(
		const input in, 
		mode_entropy entropy, 
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
					execute_player_commands(in, entropy, step);
				},
				[&](const const_logic_step step) {
					mode_post_solve(in, entropy, step);
					callbacks.post_solve(step);
				},
				callbacks.post_cleanup
			)
		);
	}
};
