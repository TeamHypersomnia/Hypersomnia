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

	if (requested.backpack) {
		const auto default_backpack = 
			associated_faction == faction_type::METROPOLIS ?
			test_container_items::METROPOLIS_BACKPACK :
			test_container_items::RESISTANCE_BACKPACK
		;

		result.back_wearable = to_entity_flavour_id(default_backpack);
	}

	if (requested.electric_armor) {
		result.armor_wearable = to_entity_flavour_id(test_tool_items::ELECTRIC_ARMOR);
	}

	result.weapon = to_flavour(requested.firearm);
	result.shoulder_wearable = to_flavour(requested.melee);
	result.num_given_ammo_pieces = requested.extra_ammo_pieces;

	if (requested.explosive.is_set()) {
		/* Security measure: put a limit on how many can be spawned */
		const auto num_explosives = std::min(20u, requested.num_explosives);

		result.other_equipment.push_back({ num_explosives, to_flavour(requested.explosive) });
	}

	return result;
}

using default_rulesets_tuple = std::tuple<test_mode_ruleset, bomb_defusal_ruleset>;

template <class F>
ruleset_id setup_ruleset_from_editor_mode(
	const editor_game_mode_resource& game_mode,
	F find_resource,
	const default_rulesets_tuple& defaults,
	const raw_ruleset_id next_id,
	predefined_rulesets& rulesets,
	const editor_arena_settings& settings,
	const editor_playtesting_settings* overrides
) {
	ruleset_id result;
	result.raw = next_id;

	auto alloc = [&]<typename T>(T& new_ruleset) {
		rulesets.all.template get<make_ruleset_map<T>>().try_emplace(next_id, std::move(new_ruleset));
	};

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
		}
	};

	auto build = [&]<typename I>(const I&) {
		if constexpr(std::is_same_v<I, editor_quick_test_mode>) {
			auto& vars = game_mode.editable.quick_test;

			using T = test_mode;

			result.type_id.set<T>();
			auto rules = std::get<typename T::ruleset_type>(defaults);
			rules.respawn_after_ms = vars.respawn_time_ms;

			::for_each_faction(
				[&](const faction_type f) {
					rules.factions[f].round_start_eq = ::to_game_requested_equipment(vars.equipment[f], find_resource, f);
				}
			);

			apply_overrides(rules);

			alloc(rules);
		}
		else if constexpr(std::is_same_v<I, editor_bomb_defusal_mode>) {
			auto& vars = game_mode.editable.bomb_defusal;

			using T = bomb_defusal;

			result.type_id.set<T>();

			auto rules = std::get<typename T::ruleset_type>(defaults);
			rules.warmup_secs = vars.warmup_time;
			rules.freeze_secs = vars.freeze_time;
			rules.round_secs = vars.round_time;
			rules.round_end_secs = vars.round_end_time;
			rules.max_rounds = (std::max(2u, vars.max_team_score) - 1) * 2;
			rules.buy_secs_after_freeze = vars.buy_time;

			::for_each_faction(
				[&](const faction_type f) {
					rules.factions[f].warmup_initial_eq = ::to_game_requested_equipment(vars.warmup_equipment[f], find_resource, f);
					rules.factions[f].round_start_eq = ::to_game_requested_equipment(vars.round_start_equipment[f], find_resource, f);
				}
			);

			apply_overrides(rules);

			alloc(rules);
		}
		else {
			static_assert(always_false_v<I>, "Non-exhaustive if constexpr");
		}
	};

	game_mode.type.dispatch(build);

	return result;
}
