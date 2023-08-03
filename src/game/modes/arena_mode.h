#pragma once
#include <unordered_map>

#include "augs/math/declare_math.h"
#include "game/modes/arena_mode_structs.h"
#include "game/detail/inventory/requested_equipment.h"
#include "game/enums/faction_type.h"
#include "game/cosmos/solvers/standard_solver.h"
#include "game/modes/mode_entropy.h"
#include "game/detail/economy/money_type.h"
#include "game/modes/mode_player_id.h"
#include "game/components/movement_component.h"
#include "game/enums/battle_event.h"
#include "augs/misc/enum/enum_array.h"
#include "augs/misc/timing/stepped_timing.h"
#include "augs/misc/timing/speed_vars.h"
#include "game/modes/mode_commands/mode_entropy_structs.h"
#include "game/detail/view_input/predictability_info.h"
#include "augs/enums/callback_result.h"
#include "game/enums/faction_choice_result.h"
#include "game/modes/session_id.h"
#include "game/modes/arena_submodes.h"

class cosmos;
struct cosmos_solvable_significant;

class arena_mode;

struct arena_mode_ruleset {
	using mode_type = arena_mode;

	// GEN INTROSPECTOR struct arena_mode_ruleset
	std::vector<entity_name_str> bot_names;
	std::vector<rgba> player_colors;

	rgba excess_player_color = orange;
	rgba default_player_color = orange;

	bool enable_player_colors = true;
	uint32_t bot_quota = 8;

	uint32_t respawn_after_ms = 0;
	uint32_t spawn_protection_ms = 0;

	uint32_t allow_spawn_for_secs_after_starting = 10;
	uint32_t max_players_per_team = 32;
	uint32_t round_secs = 120;
	uint32_t round_end_secs = 5;
	uint32_t freeze_secs = 10;
	uint32_t buy_secs_after_freeze = 30;
	uint32_t warmup_secs = 45;
	uint32_t warmup_respawn_after_ms = 2000;
	uint32_t max_rounds = 30;
	uint32_t match_summary_seconds = 15;
	uint32_t game_commencing_seconds = 3;
	meter_value_type minimal_damage_for_assist = 41;
	per_actual_faction<arena_mode_faction_rules> factions;

	constrained_entity_flavour_id<invariants::explosive, invariants::hand_fuse> bomb_flavour;
	bool delete_lying_items_on_round_start = false;
	bool delete_lying_items_on_warmup = true;
	bool allow_game_commencing = true;
	bool refill_all_mags_on_round_start = true;
	bool refill_chambers_on_round_start = true;
	bool allow_spectator_to_see_both_teams = true;
	bool forbid_going_to_spectator_unless_character_alive = true;
	bool allow_spectate_enemy_if_no_conscious_players = true;
	bool hide_details_when_spectating_enemies = true;

	bool enable_item_shop = true;
	bool warmup_enable_item_shop = false;

	arena_mode_economy_rules economy;
	arena_mode_view_rules view;

	augs::speed_vars speeds;

	all_subrules_variant subrules;
	// END GEN INTROSPECTOR

	arena_mode_ruleset();

	bool should_hide_details_when_spectating_enemies() const {
		return true;
	}

	auto get_num_rounds() const {
		/* Make it even */
		return std::max((max_rounds / 2) * 2, 2u);
	}

	bool has_economy() const;
	bool has_bomb_mechanics() const { return bomb_flavour.is_set(); }
};

struct arena_mode_faction_state {
	// GEN INTROSPECTOR struct arena_mode_faction_state
	uint32_t current_spawn_index = 0;
	uint32_t score = 0;
	uint32_t consecutive_losses = 0;
	std::vector<mode_entity_id> shuffled_spawns;
	// END GEN INTROSPECTOR

	void clear_for_next_half() {
		current_spawn_index = 0;
		consecutive_losses = 0;
		shuffled_spawns.clear();
	}
};

struct arena_mode_player_stats {
	// GEN INTROSPECTOR struct arena_mode_player_stats
	money_type money = 0;

	int knockout_streak = 0;

	int knockouts = 0;
	int assists = 0;
	int deaths = 0;

	int bomb_plants = 0;
	int bomb_explosions = 0;

	int bomb_defuses = 0;

	int level = 0;

	arena_mode_player_round_state round_state;
	// END GEN INTROSPECTOR

	int calc_score() const;
};

struct arena_mode_player {
	// GEN INTROSPECTOR struct arena_mode_player
	player_session_data session;
	entity_id controlled_character_id;
	rgba assigned_color = rgba::zero;
	arena_mode_player_stats stats;
	uint32_t round_when_chosen_faction = static_cast<uint32_t>(-1); 

	bool is_bot = false;
	// END GEN INTROSPECTOR

	arena_mode_player(const entity_name_str& nickname = {}) {
		session.nickname = nickname;
	}

	bool operator<(const arena_mode_player& b) const;

	bool is_set() const {
		return session.is_set();
	}

	const auto& get_nickname() const {
		return session.nickname;
	}

	const auto& get_faction() const {
		return session.faction;
	}

	const auto& get_session_id() const {
		return session.id;
	}

	auto should_hide_in_scoreboard() const {
		return false;
	}

	auto get_order() const {
		return arena_player_order_info { get_nickname(), stats.calc_score() };
	}
};

struct arena_mode_round_state {
	// GEN INTROSPECTOR struct arena_mode_round_state
	bool cache_players_frozen = false;
	arena_mode_win last_win;
	arena_mode_knockouts_vector knockouts;
	mode_player_id bomb_planter;
	// END GEN INTROSPECTOR
};

enum class round_start_type {
	KEEP_EQUIPMENTS,
	DONT_KEEP_EQUIPMENTS
};

struct debugger_property_accessors;

class arena_mode {
public:
	using ruleset_type = arena_mode_ruleset;
	using player_type = arena_mode_player;

	static constexpr bool needs_clean_round_state = true;

	template <bool C>
	struct basic_input {
		using mode_type = arena_mode;

		const ruleset_type& rules;
		const cosmos_solvable_significant& clean_round_state;
		maybe_const_ref_t<C, cosmos> cosm;

		template <bool is_const = C, class = std::enable_if_t<!is_const>>
		operator basic_input<!is_const>() const {
			return { rules, clean_round_state, cosm };
		}
	};

	using input = basic_input<false>;
	using const_input = basic_input<true>;

	struct participating_factions {
		faction_type bombing = faction_type::SPECTATOR;
		faction_type defusing = faction_type::SPECTATOR;

		bool valid() const {
			return bombing != faction_type::SPECTATOR && defusing != faction_type::SPECTATOR;
		}

		static auto fallback() {
			return participating_factions { faction_type::RESISTANCE, faction_type::METROPOLIS };
		}

		template <class F>
		void for_each(F callback) const {
			callback(bombing);
			callback(defusing);
		}

		std::size_t size() const {
			return 2;
		}

		auto get_all() const {
			augs::constant_size_vector<faction_type, std::size_t(faction_type::COUNT)> result;
			for_each([&result](const auto f) { result.push_back(f); });
			return result;
		}

		faction_type get_swapped(faction_type f) const {
			if (f == bombing) {
				return defusing;
			}

			return bombing;
		}
	};

	participating_factions calc_participating_factions(const_input) const;
	faction_type calc_weakest_faction(const_input) const;

	bool is_halfway_round(const_input) const;
	bool is_final_round(const_input) const;

	arena_mode_player_stats* stats_of(const mode_player_id&);

private:
	struct transferred_inventory {
		struct item {
			constrained_entity_flavour_id<components::item> flavour;

			int charges = -1;
			item_owner_meta owner_meta;
			std::size_t container_index = static_cast<std::size_t>(-1);
			slot_function slot_type = slot_function::INVALID;
			entity_id source_entity_id;
		};

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

	void count_win(input, const_logic_step, faction_type);

	void standard_victory(input, const_logic_step, faction_type, bool announce = true, bool play_theme = true);

	entity_id create_character_for_player(input, logic_step, mode_player_id, std::optional<transfer_meta> = std::nullopt);

	template <class H>
	void reset_equipment_for(logic_step step, input in, mode_player_id ko, H character_handle);

	void teleport_to_next_spawn(input, entity_id character);
	void init_spawned(
		input, 
		mode_player_id,
		entity_id character, 
		logic_step, 
		std::optional<transfer_meta> = std::nullopt
	);

	void mode_pre_solve(input, const mode_entropy&, logic_step);
	void mode_post_solve(input, const mode_entropy&, logic_step);

	void start_next_round(input, logic_step, round_start_type = round_start_type::KEEP_EQUIPMENTS);
	void setup_round(input, logic_step, const round_transferred_players& = {});
	void reshuffle_spawns(const cosmos&, faction_type);

	void set_players_frozen(input in, bool flag);
	void release_triggers_of_weapons_of_players(input in);

	void respawn_the_dead(input, logic_step, unsigned after_ms);

	bool bomb_exploded(const const_input) const;
	entity_id get_character_who_defused_bomb(const_input) const;
	bool bomb_planted(const_input) const;

	void play_start_round_sound(input, const_logic_step);

	void play_faction_sound(const_logic_step, faction_type, assets::sound_id, predictability_info) const;
	void play_faction_sound_for(input, const_logic_step, battle_event, faction_type, predictability_info) const;

	void play_sound_for(input, const_logic_step, battle_event, predictability_info) const;
	void play_win_sound(input, const_logic_step, faction_type) const;
	void play_win_theme(input, const_logic_step, faction_type) const;

	void play_sound_globally(const_logic_step, assets::sound_id, predictability_info) const;

	void play_bomb_defused_sound(input, const_logic_step, faction_type) const;

	void process_win_conditions(input, logic_step);

	std::size_t get_round_rng_seed(const cosmos&) const;
	std::size_t get_step_rng_seed(const cosmos&) const;

	void count_knockout(logic_step, input, entity_id victim, const components::sentience&);
	void count_knockout(logic_step, input, arena_mode_knockout);

	entity_handle spawn_bomb(input);
	bool give_bomb_to_random_player(input, logic_step);
	void spawn_bomb_near_players(input);

	void end_warmup_and_go_live(input, logic_step);

	void execute_player_commands(input, mode_entropy&, logic_step);
	void add_or_remove_players(input, const mode_entropy&, logic_step);
	void handle_special_commands(input, const mode_entropy&, logic_step);
	void spawn_characters_for_recently_assigned(input, logic_step);
	void spawn_and_kick_bots(input, logic_step);

	void handle_game_commencing(input, logic_step);

	template <class S, class E>
	static auto find_player_by_impl(S& self, const E& identifier);

	/* A server might provide its own integer-based identifiers */
	bool add_player_custom(input, const add_player_input&);

	void remove_player(input, logic_step, const mode_player_id&);
	mode_player_id add_player(input, const entity_name_str& nickname);

	faction_choice_result auto_assign_faction(input, const mode_player_id&);
	faction_choice_result choose_faction(const_input, const mode_player_id&, const faction_type faction);

	void restart_match(input, logic_step);

	void reset_players_stats(input);
	void set_players_money_to_initial(input);
	void set_players_level_to_initial(input);
	void clear_players_round_state(input);

	void give_monetary_award(input, mode_player_id, money_type amount);

	template <class C, class F>
	void for_each_player_handle_in(C&, faction_type, F&& callback) const;

	// GEN INTROSPECTOR class arena_mode
	arena_mode_state state = arena_mode_state::INIT;

	per_actual_faction<arena_mode_faction_state> factions;
	std::map<mode_player_id, player_type> players;
	arena_mode_round_state current_round;

	augs::stepped_clock clock_before_setup;
	real32 commencing_timer_ms = -1.f;

	bool had_first_blood = false;
	bool should_commence_when_ready = false;

	uint32_t current_num_bots = 0;
	augs::speed_vars round_speeds;
	session_id_type next_session_id = session_id_type::first();
	uint32_t scramble_counter = 0;
	uint32_t prepare_to_fight_counter = 0;

	mode_player_id duellist_1 = mode_player_id::dead();
	mode_player_id duellist_2 = mode_player_id::dead();

	entity_id bomb_entity;
	entity_id bomb_detonation_theme;
	// END GEN INTROSPECTOR

	friend augs::introspection_access;
	friend debugger_property_accessors;
	friend class editor_setup;

	void on_faction_changed_for(const_input, faction_type previous_faction, const mode_player_id&);
	void assign_free_color_to_best_uncolored(const_input in, faction_type previous_faction, rgba free_color);

	void swap_assigned_factions(const participating_factions&);
	void scramble_assigned_factions(const participating_factions&);

public:

	faction_type get_player_faction(const mode_player_id&) const;
	faction_type get_opposing_faction(const_input, faction_type) const;

	mode_entity_id lookup(const mode_player_id&) const;
	mode_player_id lookup(const mode_entity_id&) const;

	unsigned get_current_round_number() const;
	bool is_first_round_in_half(const const_input in) const;

	float get_seconds_passed_in_cosmos(const_input) const;

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

	mode_player_id find_first_free_player() const;

	arena_mode_match_result calc_match_result(const_input) const;

	unsigned get_score(faction_type) const;

	player_type* find_player_by(const entity_name_str& nickname);
	player_type* find(const mode_player_id&);
	const player_type* find_player_by(const entity_name_str& nickname) const;
	const player_type* find(const mode_player_id&) const;

	const player_type* find(const session_id_type&) const;
	mode_player_id lookup(const session_id_type&) const;

	template <class F>
	void for_each_player_in(faction_type, F&& callback) const;

	template <class F>
	void for_each_player_best_to_worst_in(faction_type, F&& callback) const;

	mode_player_id find_best_player_in(faction_type) const;

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
				[&](const logic_step step) {
					mode_post_solve(in, entropy, step);
					callbacks.post_solve(const_logic_step(step));
				},
				callbacks.post_cleanup
			)
		);
	}

	const auto& get_round_speeds() const {
		return round_speeds;
	}

	auto get_commencing_left_ms() const {
		return commencing_timer_ms;
	}

	bool is_game_commencing() const {
		return commencing_timer_ms != -1.0f;
	}

	const auto& get_current_round() const {
		return current_round;
	}

	const auto& get_players() const {
		return players;
	}

	auto get_state() const {
		return state;
	}

	bool is_match_summary() const {
		return state == arena_mode_state::MATCH_SUMMARY;
	}

	uint32_t get_faction_score(const faction_type faction) const {
		return factions[faction].score;
	}

	mode_player_id get_next_to_spectate(
		const_input, 
		const arena_player_order_info& after, 
		const faction_type& by_faction, 
		int offset,
		real32 limit_in_seconds
	) const;

	bool suitable_for_spectating(
		const_input, 
		const mode_player_id& who, 
		const mode_player_id& by, 
		real32 limit_in_seconds
	) const;
	bool conscious_or_can_still_spectate(const_input, const mode_player_id& who, real32 limit_in_seconds) const;

	template <class C, class F>
	decltype(auto) on_player_handle(C& cosm, const mode_player_id& id, F&& callback) const {
		if (const auto player_data = find(id)) {
			if (const auto handle = cosm[player_data->controlled_character_id]) {
				return callback(handle);
			}
		}

		return callback(std::nullopt);
	}

	template <class F>
	decltype(auto) on_bomb_entity(const const_input in, F callback) const;

	augs::maybe<rgba> get_current_fallback_color_for(const_input, faction_type faction) const;

	template <class F>
	void for_each_player_id(F callback) const {
		for (const auto& p : players) {
			if (callback(p.first) == callback_result::ABORT) {
				return;
			}
		}
	}

	auto get_next_session_id() const {
		return next_session_id;
	}

	arena_migrated_session emigrate() const;
	void migrate(input, const arena_migrated_session&);

	std::size_t num_conscious_players_in(const cosmos&, faction_type) const;
	std::size_t num_players_in(faction_type) const;

	uint32_t get_num_players() const;
	uint32_t get_num_active_players() const;

	uint32_t get_max_num_active_players(const_input) const;

	void check_duel_of_honor(input, logic_step);
	void post_team_match_start(input, logic_step);

	bool is_a_duellist(const mode_player_id&) const;
	mode_player_id get_opponent_duellist(const mode_player_id&) const;
	void clear_duel();

	void handle_duel_desertion(input, logic_step, const mode_player_id&);
	void report_match_result(input, const_logic_step);

	game_mode_name_type get_name(const_input) const;

	bool can_respawn_already() const;

	bool levelling_enabled(const_input) const;
};
