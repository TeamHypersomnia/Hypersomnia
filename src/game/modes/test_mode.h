#pragma once
#include <cstddef>
#include <cstdint>
#include "augs/math/declare_math.h"
#include "game/detail/inventory/requested_equipment.h"
#include "game/enums/faction_type.h"
#include "game/cosmos/solvers/standard_solver.h"
#include "game/modes/mode_entropy.h"
#include "game/modes/mode_player_id.h"
#include "game/modes/detail/fog_of_war_settings.h"
#include "view/character_hud_type.h"

#include "game/modes/session_id.h"
#include "game/modes/arena_mode_structs.h"
#include "augs/templates/continue_or_callback_result.h"
#include "game/modes/ranked_state_type.h"

#include "application/arena/arena_playtesting_context.h"

using mode_entity_id = entity_id;

struct entity_id;

class cosmos;

struct test_mode_faction_rules {
	// GEN INTROSPECTOR struct test_mode_faction_rules
	requested_equipment round_start_eq;
	// END GEN INTROSPECTOR
};

struct test_mode_view_rules {
	// GEN INTROSPECTOR struct test_mode_view_rules
	double audiovisual_speed = 1.0;
	fog_of_war_settings fog_of_war;
	character_hud_type enemy_hud_mode = character_hud_type::SMALL_HEALTH_BAR;

	bool enable_danger_indicators = true;
	bool enable_teammate_indicators = true;
	bool enable_tactical_indicators = true;
	// END GEN INTROSPECTOR
};

class test_mode;

struct test_mode_ruleset {
	using mode_type = test_mode;

	// GEN INTROSPECTOR struct test_mode_ruleset
	std::string name = "Unnamed test scene mode vars";

	real32 respawn_after_ms = 3000;
	per_actual_faction<test_mode_faction_rules> factions;
	test_mode_view_rules view;
	uint32_t round_secs = 120;
	// END GEN INTROSPECTOR

	test_mode_ruleset();

	bool should_hide_details_when_spectating_enemies() const {
		return true;
	}

	bool is_ffa() const {
		return false;
	}
};

struct test_mode_player_stats {
	// GEN INTROSPECTOR struct test_mode_player_stats
	money_type money = 0;

	int knockouts = 0;
	int assists = 0;
	int deaths = 0;
	// END GEN INTROSPECTOR

	int calc_score() const { 
		return 
			knockouts * 2 
			+ assists
		;
	}
};

struct test_mode_player {
	// GEN INTROSPECTOR struct test_mode_player
	player_session_data session;
	mode_entity_id controlled_character_id;
	test_mode_player_stats stats;
	entity_id dedicated_spawn;

	bool hide_in_scoreboard = false;
	bool allow_respawn = true;
	// END GEN INTROSPECTOR

	/* Dummy field for compatibility */
	std::string server_ranked_account_id;

	auto should_hide_in_scoreboard() const {
		return hide_in_scoreboard;
	}

	bool operator<(const test_mode_player& b) const;

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

	auto get_score() const {
		return stats.calc_score();
	}

	auto get_level() const {
		return 0;
	}

	test_mode_player(const mode_entity_id controlled_character_id = mode_entity_id()) : controlled_character_id(controlled_character_id) {}
};

struct synced_dynamic_vars;

class test_mode {
public:
	using ruleset_type = test_mode_ruleset;
	using player_type = test_mode_player;

	static constexpr bool needs_clean_round_state = false;

	template <bool C>
	struct basic_input {
		using mode_type = test_mode;

		const synced_dynamic_vars& dynamic_vars;
		const ruleset_type& rules;
		maybe_const_ref_t<C, cosmos> cosm;

		template <bool is_const = C, class = std::enable_if_t<!is_const>>
		operator basic_input<!is_const>() const {
			return { dynamic_vars, rules, cosm };
		}
	};

	using input = basic_input<false>;
	using const_input = basic_input<true>;

	void teleport_to_next_spawn(input, mode_player_id, entity_id character);
private:
	void init_spawned(input, entity_id character, logic_step);

	void mode_pre_solve(input, const mode_entropy&, logic_step);

	void create_controlled_character_for(input in, mode_player_id);

public:
	// GEN INTROSPECTOR class test_mode
	unsigned current_spawn_index = 0;
	std::vector<mode_entity_id> pending_inits;
	std::map<mode_player_id, player_type> players;

	session_id_type next_session_id = session_id_type::first();
	std::optional<arena_playtesting_context> playtesting_context;
	entity_id infinite_ammo_for;
	augs::speed_vars round_speeds;
	// END GEN INTROSPECTOR

	mode_player_id add_player(input, const entity_name_str& nickname, const faction_type);
	void remove_player(input, logic_step, mode_player_id);

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
				[&](const logic_step step) {
					mode_post_solve(in, entropy, step);
					callbacks.post_solve(const_logic_step(step));
				},
				callbacks.post_cleanup
			)
		);
	}

	void mode_post_solve(input, const mode_entropy&, logic_step);

	bool add_player_custom(input, const add_player_input&);
	void add_or_remove_players(input, const mode_entropy&, logic_step);

	arena_migrated_session emigrate() const;
	void migrate(input, const arena_migrated_session&);

	auto get_next_session_id() const {
		return next_session_id;
	}

	template <class S, class E>
	static auto find_player_by_impl(S& self, const E& identifier);

	player_type* find_player_by(const entity_name_str& nickname);
	player_type* find(const mode_player_id&);
	const player_type* find_player_by(const entity_name_str& nickname) const;
	const player_type* find(const mode_player_id&) const;

	const player_type* find_suspended(const mode_player_id&) const {
		return nullptr;
	}

	const player_type* find(const session_id_type&) const;
	mode_player_id lookup(const session_id_type&) const;

	template <class F>
	void for_each_player(F callback) {
		for (auto& p : players) {
			if (callback(p.first, p.second) == callback_result::ABORT) {
				return;
			}
		}
	}

	template <class F>
	void for_each_player_id(F callback) const {
		for (const auto& p : players) {
			if (callback(p.first) == callback_result::ABORT) {
				return;
			}
		}
	}

	template <class F>
	void for_each_player_in(const faction_type faction, F&& callback) const {
		for (auto& it : players) {
			if (it.second.get_faction() == faction) {
				if (::continue_or_callback_result(std::forward<F>(callback), it.first, it.second) == callback_result::ABORT) {
					return;
				}
			}
		}
	}

	std::size_t num_players_in(faction_type) const;

	uint32_t get_num_players() const;
	uint32_t get_num_active_players() const;

	uint32_t get_max_num_active_players(const_input) const;

	uint32_t calc_max_faction_score() const {
		return 0;
	}

	uint32_t get_faction_score(const faction_type) const {
		return 0;
	}

	void remove_old_lying_items(input, logic_step);

	game_mode_name_type get_name(const_input) const {
		return "Test mode";
	}

	bool levelling_enabled(const_input) const {
		return false;
	}

	const auto& get_players() const {
		return players;
	}

	float get_seconds_passed_in_cosmos(const_input) const;
	float get_round_seconds_left(const_input) const;

	auto get_ranked_state() const {
		return ranked_state_type::NONE;
	}

	bool should_suspend_instead_of_remove(const_input) const {
		return false;
	}

	std::size_t num_suspended_players() const {
		return 0;
	};

	mode_player_id find_suspended_player_id(const std::string&) const {
		return mode_player_id::dead();
	}

	bool can_use_map_command_now(const const_input) const {
		return true;
	}

	bool is_idle() const {
		return players.empty();
	}

	bool team_choice_allowed(const_input) const {
		return true;
	}

	uint32_t continuous_sounds_clock(const_input in) const {
		return in.cosm.get_total_steps_passed();
	}

	uint32_t get_num_bot_players() const {
		return 0u;
	}

	uint32_t get_num_human_players() const {
		return get_num_players() - get_num_bot_players();
	}
};
