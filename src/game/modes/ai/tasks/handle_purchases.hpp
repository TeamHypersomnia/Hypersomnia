#pragma once
#include <optional>
#include "augs/misc/randomization.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/modes/detail/item_purchase_logic.hpp"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/detail/weapon_like.h"

inline std::optional<item_flavour_id> handle_purchases(
	const ai_character_context& ctx,
	const money_type money,
	const float dt_secs,
	randomization& stable_rng,
	const arena_shop_whitelists& shop_whitelists
) {
	if (money <= 0) {
		ctx.ai_state.already_nothing_more_to_buy = true;
	}

	if (ctx.ai_state.already_nothing_more_to_buy) {
		return std::nullopt;
	}

	if (ctx.ai_state.purchase_decision_countdown < 0.0f) {
		if (ctx.ai_state.purchase_decision_countdown == -10000.0f) {
			ctx.ai_state.purchase_decision_countdown = stable_rng.randval(1.0f, 2.5f);
		}
		else {
			/* Make subsequent purchases quicker */
			ctx.ai_state.purchase_decision_countdown = stable_rng.randval(0.3f, 0.7f);
		}
	}

	const auto buyer_faction = ctx.character_handle.get_official_faction();
	ctx.ai_state.purchase_decision_countdown -= dt_secs;

	/*
		We need to check inventory now to determine if we should buy anything.
		This allows us to set already_nothing_more_to_buy even during countdown
		if we determine there's nothing to purchase.
	*/
	auto get_owned_inventory = [&]() {
		bool has_only_pistols = true;
		int weapon_count = 0;
		std::vector<item_flavour_id> owned_guns;
		bool has_backpack = false;
		bool has_defuse_kit = false;
		bool has_armor = false;

		ctx.character_handle.for_each_contained_item_recursive(
			[&](const auto& item) {
				if (const auto gun = item.template find<invariants::gun>()) {
					weapon_count++;
					owned_guns.push_back(item.get_flavour_id());

					if (gun->buy_type != buy_menu_type::PISTOLS) {
						has_only_pistols = false;
					}
				}

				if (::is_backpack_like(item)) {
					has_backpack = true;
				}

				if (const auto tool = item.template find<invariants::tool>()) {
					if (tool->defusing_speed_mult > 1.f) {
						has_defuse_kit = true;
					}
				}

				if (::is_armor_like(item)) {
					has_armor = true;
				}
			}
		);

		return std::tuple{ weapon_count, has_only_pistols, owned_guns, has_backpack, has_defuse_kit, has_armor };
	};

	/*
		Every item this bot could buy with available_money: its faction's, sold in this map's shop, with a price.
	*/

	auto for_each_affordable = [&](const money_type available_money, auto&& callback) {
		ctx.cosm.for_each_flavour_having<invariants::item>([&](const auto& id, const auto& flavour) {
			const auto item_id = item_flavour_id(id);

			if (!factions_compatible(ctx.character_handle, id)) {
				return;
			}

			if (!::is_sold_in_shop(ctx.cosm, shop_whitelists, item_id)) {
				return;
			}

			const auto price = ::find_price_of(ctx.cosm, item_id);

			if (!price.has_value() || *price <= 0 || *price > available_money) {
				return;
			}

			callback(item_id, flavour, *price);
		});
	};

	auto try_buy_cheapest = [&](const money_type available_money, auto&& predicate) -> std::optional<item_flavour_id> {
		std::optional<item_flavour_id> cheapest;
		money_type cheapest_price = std::numeric_limits<money_type>::max();

		for_each_affordable(available_money, [&](const item_flavour_id& id, const auto& flavour, const money_type price) {
			if (predicate(flavour) && price < cheapest_price) {
				cheapest_price = price;
				cheapest = id;
			}
		});

		return cheapest;
	};

	auto try_buy_backpack = [&](const money_type available_money) {
		return try_buy_cheapest(available_money, [](const auto& flavour) {
			return ::is_backpack_like(flavour);
		});
	};

	auto try_buy_defuse_kit = [&](const money_type available_money) {
		return try_buy_cheapest(available_money, [](const auto& flavour) {
			if (const auto tool = flavour.template find<invariants::tool>()) {
				return tool->defusing_speed_mult > 1.f;
			}

			return false;
		});
	};

	auto try_buy_gun = [&](const std::vector<item_flavour_id>& owned_guns, const money_type available_money) -> std::optional<item_flavour_id> {
		std::vector<item_flavour_id> affordable_pistols;
		std::vector<item_flavour_id> affordable_non_pistols;

		for_each_affordable(available_money, [&](const item_flavour_id& id, const auto& flavour, money_type) {
			if (const auto gun = flavour.template find<invariants::gun>()) {
				if (gun->bots_ban) {
					return;
				}

				if (gun->buy_type == buy_menu_type::PISTOLS) {
					affordable_pistols.push_back(id);
				}
				else {
					affordable_non_pistols.push_back(id);
				}
			}
		});

		/*
			With pistols only, just a rifle is worth buying - another pistol would only push one out over the limit.
		*/

		const auto& weapons_to_choose_from =
			!affordable_non_pistols.empty() || !owned_guns.empty() ? affordable_non_pistols : affordable_pistols
		;

		if (weapons_to_choose_from.empty()) {
			return std::nullopt;
		}

		const auto random_index = stable_rng.randval(0u, static_cast<unsigned>(weapons_to_choose_from.size() - 1));
		const auto bought = weapons_to_choose_from[random_index];

		if (found_in(owned_guns, bought)) {
			return std::nullopt;
		}

		return bought;
	};

	auto try_buy_armor = [&](const money_type available_money) -> std::optional<item_flavour_id> {
		std::vector<item_flavour_id> affordable_armor;

		for_each_affordable(available_money, [&](const item_flavour_id& id, const auto& flavour, money_type) {
			if (flavour.template find<invariants::tool>() != nullptr && ::is_armor_like(flavour)) {
				affordable_armor.push_back(id);
			}
		});

		if (affordable_armor.empty()) {
			return std::nullopt;
		}

		const auto random_index = stable_rng.randval(0u, static_cast<unsigned>(affordable_armor.size() - 1));
		return affordable_armor[random_index];
	};

	const auto [weapon_count, has_only_pistols, owned_guns, has_backpack, has_defuse_kit, has_armor] = get_owned_inventory();

	const bool buy_gun = weapon_count == 0 || has_only_pistols;

	/*
		Buy gun if needed.
	*/
	if (buy_gun) {
		if (const auto gun = try_buy_gun(owned_guns, money)) {
			/*
				Countdown still delays the actual purchase.
			*/
			if (ctx.ai_state.purchase_decision_countdown > 0.0f) {
				return std::nullopt;
			}

			return gun;
		}
	}

	/*
		Buy armor if nothing else and we don't already have armor.
	*/
	if (!has_armor) {
		if (const auto armor = try_buy_armor(money)) {
			/*
				Countdown still delays the actual purchase.
			*/
			if (ctx.ai_state.purchase_decision_countdown > 0.0f) {
				return std::nullopt;
			}

			return armor;
		}
	}

	/*
		If we have >=10000 money and don't need to buy anything else, buy a defuse kit, then a backpack.
		
		The 10000 threshold ensures bots prioritize essential equipment first.
		Defuse kits and backpacks are useful but not critical - only buy when we have excess money.
		A gun never waits for a backpack - buying it over the firearm limit drops the cheapest one.
	*/

	constexpr money_type spare_money_threshold_v = 10000;

	if (buyer_faction == faction_type::METROPOLIS) {
		if (money >= spare_money_threshold_v && !has_defuse_kit) {
			if (const auto kit = try_buy_defuse_kit(money)) {
				/*
					Countdown still delays the actual purchase.
				*/
				if (ctx.ai_state.purchase_decision_countdown > 0.0f) {
					return std::nullopt;
				}

				return kit;
			}
		}
	}

	if (money >= spare_money_threshold_v && !has_backpack) {
		if (const auto backpack = try_buy_backpack(money)) {
			/*
				Countdown still delays the actual purchase.
			*/
			if (ctx.ai_state.purchase_decision_countdown > 0.0f) {
				return std::nullopt;
			}

			return backpack;
		}
	}

	/*
		Nothing more to buy - set flag so we don't keep checking.
		This can be set even during countdown if we determine there's nothing to purchase.
	*/
	ctx.ai_state.already_nothing_more_to_buy = true;

	return std::nullopt;
}
