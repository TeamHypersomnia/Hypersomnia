#pragma once
#include "augs/templates/remove_cref.h"

template <class F>
void for_each_faction(F callback) {
	callback(faction_type::METROPOLIS);
	callback(faction_type::RESISTANCE);
	callback(faction_type::ATLANTIS);
}

template <class F>
auto to_game_requested_equipment(
	const editor_requested_equipment& requested,
	F find_resource,
	faction_type associated_faction = faction_type::SPECTATOR
) {
	auto to_flavour = [&]<typename R>(const editor_typed_resource_id<R> typed_id) -> 
		remove_cref<decltype(std::get<0>(std::declval<R>().scene_flavour_id))> 
	{
		if (auto res = find_resource(typed_id)) {
			return std::get<0>(res->scene_flavour_id);
		}

		return {};
	};

	requested_equipment result;
	result.personal_deposit_wearable = to_entity_flavour_id(test_container_items::STANDARD_PERSONAL_DEPOSIT);
	result.haste_time = requested.haste_time;

	if (requested.backpack) {
		const auto default_backpack = 
			associated_faction == faction_type::METROPOLIS ?
			test_tool_items::METROPOLIS_BACKPACK :
			test_tool_items::RESISTANCE_BACKPACK
		;

		result.back_wearable = to_entity_flavour_id(default_backpack);
	}

	if (requested.electric_armor) {
		result.armor_wearable = to_entity_flavour_id(test_tool_items::ELECTRIC_ARMOR);
	}

	result.weapon = to_flavour(requested.firearm);
	result.shoulder_wearable = to_flavour(requested.melee);

	if (!result.weapon.is_set()) {
		std::swap(result.shoulder_wearable, result.weapon);
	}

	result.num_given_ammo_pieces = requested.extra_ammo_pieces;

	if (requested.explosive.is_set()) {
		/* Security measure: put a limit on how many can be spawned */
		const auto num_explosives = std::min(20u, requested.num_explosives);

		result.other_equipment.push_back({ num_explosives, to_flavour(requested.explosive) });
	}

	fill_range(result.spells_to_give, requested.all_spells);

	return result;
}

using default_rulesets_tuple = std::tuple<test_mode_ruleset, arena_mode_ruleset>;

template <class T>
constexpr bool is_arena_submode_v = is_one_of_v<T, editor_bomb_defusal_mode, editor_gun_game_mode>;

template <class F>
auto setup_ruleset_from_editor_mode(
	const editor_game_mode_resource& game_mode,
	F find_resource,
	const editor_arena_settings& settings,
	const editor_playtesting_settings* overrides
) {
	auto to_flavour = [&]<typename R>(const editor_typed_resource_id<R> typed_id) -> 
		remove_cref<decltype(std::get<0>(std::declval<R>().scene_flavour_id))> 
	{
		if (auto res = find_resource(typed_id)) {
			return std::get<0>(res->scene_flavour_id);
		}

		return {};
	};

	all_rulesets_variant result;

	auto apply_global_settings = [&]<typename T>(T& rules) {
		if constexpr(std::is_same_v<T, test_mode_ruleset>) {

		}
		else {
			/* TODO: warmup_theme needs to be a sound_effect input for gain/pitch params to have any effect at all! */
			auto to_flavour = [&]<typename R>(const editor_typed_resource_id<R> typed_id) -> 
				remove_cref<decltype(std::get<0>(std::declval<R>().scene_flavour_id))> 
			{
				if (auto res = find_resource(typed_id)) {
					return std::get<0>(res->scene_flavour_id);
				}

				return {};
			};

			rules.view.warmup_theme = to_flavour(settings.warmup_theme.id);
		}
	};

	auto apply_overrides = [&]<typename T>(T& rules) {
		apply_global_settings(rules);

		if (overrides == nullptr) {
			return;
		}

		if constexpr(std::is_same_v<T, test_mode_ruleset>) {

		}
		else {
			if (overrides->skip_warmup) {
				rules.warmup_secs = 0;
			}

			if (overrides->skip_freeze_time) {
				rules.freeze_secs = 0;
			}

			if (overrides->unlimited_money) {
				rules.economy.initial_money = 40000;
				rules.economy.maximum_money = 40000;
			}

			rules.bot_quota = overrides->bots;
		}
	};

	auto make_arena_mode_rules = [&]<typename I>(const I& vars) {
		constexpr bool is_gun_game_v = std::is_same_v<I, editor_gun_game_mode>;

		using T = arena_mode;

		auto rules = typename T::ruleset_type();

		rules.warmup_secs = vars.warmup_time;
		rules.freeze_secs = vars.freeze_time;
		rules.round_secs = vars.round_time;
		rules.round_end_secs = vars.round_end_time;
		rules.respawn_after_ms = vars.respawn_after_ms;
		rules.spawn_protection_ms = vars.spawn_protection_ms;
		rules.max_rounds = (std::max(2u, vars.max_team_score) - 1) * 2;

		using S = typename I::subrules_type;
		S subrules;

		::for_each_faction(
			[&](const faction_type f) {
				rules.factions[f].warmup_initial_eq = ::to_game_requested_equipment(vars.warmup_equipment[f], find_resource, f);

				if constexpr(is_gun_game_v) {
					subrules.can_throw_melee_on_final_level = vars.can_throw_melee_on_final_level;

					subrules.basic_eq[f] = ::to_game_requested_equipment(vars.basic_equipment[f], find_resource, f);
					subrules.final_eq[f] = ::to_game_requested_equipment(vars.final_equipment[f], find_resource, f);

					auto& basic_eq = subrules.basic_eq[f];

					if (basic_eq.weapon.is_set()) {
						/* 
							The weapon field will be overridden by the weapon for a given level.
							This allows specifying a secondary weapon just in case.
						*/

						basic_eq.other_equipment.push_back({ 1, basic_eq.weapon });
						basic_eq.weapon = {};
					}

					rules.delete_lying_items_on_round_start = true;
				}
				else {
					rules.factions[f].round_start_eq = ::to_game_requested_equipment(vars.round_start_equipment[f], find_resource, f);
				}
			}
		);

		if constexpr(subrules.has_economy()) {
			rules.buy_secs_after_freeze = vars.buy_time;
		}

		if constexpr(is_gun_game_v) {
			for (const auto& firearm : vars.progression) {
				subrules.progression.push_back(to_flavour(firearm));
			}

			rules.bomb_flavour = {};

			rules.view.show_info_icons_of_opponents = true;
			rules.view.show_money_of_opponents = true;

			/* Always spawn */
			rules.allow_spawn_for_secs_after_starting = static_cast<uint32_t>(-1);

			if (rules.respawn_after_ms <= 0.0f) {
				rules.respawn_after_ms = 20.0f;
			}
		}

		rules.subrules = subrules;
		apply_overrides(rules);
		result = rules;
	};

	auto build = [&]<typename I>(const I&) {
		if constexpr(std::is_same_v<I, editor_quick_test_mode>) {
			auto& vars = game_mode.editable.quick_test;

			using T = test_mode;

			auto rules = typename T::ruleset_type();

			rules.respawn_after_ms = vars.respawn_time_ms;

			::for_each_faction(
				[&](const faction_type f) {
					rules.factions[f].round_start_eq = ::to_game_requested_equipment(vars.equipment[f], find_resource, f);
				}
			);

			apply_overrides(rules);

			result = rules;
		}
		else if constexpr(std::is_same_v<I, editor_gun_game_mode>) {
			make_arena_mode_rules(game_mode.editable.gun_game);
		}
		else if constexpr(std::is_same_v<I, editor_bomb_defusal_mode>) {
			make_arena_mode_rules(game_mode.editable.bomb_defusal);
		}
		else {
			static_assert(always_false_v<I>, "Non-exhaustive if constexpr");
		}
	};

	game_mode.type.dispatch(build);

	return result;
}
