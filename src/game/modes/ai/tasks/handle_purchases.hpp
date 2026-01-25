#pragma once
#include <optional>
#include "augs/misc/randomization.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/modes/detail/item_purchase_logic.hpp"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/arena_mode_ai_structs.h"

inline std::optional<item_flavour_id> handle_purchases(
	const ai_character_context& ctx,
	const money_type money,
	const float dt_secs,
	randomization& stable_rng
) {
	if (ctx.ai_state.already_nothing_more_to_buy || money <= 0) {
		return std::nullopt;
	}

	if (ctx.ai_state.purchase_decision_countdown < 0.0f) {
		ctx.ai_state.purchase_decision_countdown = stable_rng.randval(1.0f, 3.0f);
	}

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
			}
		);

		return std::tuple{ weapon_count, has_only_pistols, owned_guns, has_backpack, has_defuse_kit };
	};

	auto try_buy_backpack = [&](money_type available_money) -> std::optional<item_flavour_id> {
		std::optional<item_flavour_id> cheapest_backpack;
		money_type cheapest_price = std::numeric_limits<money_type>::max();

		ctx.cosm.for_each_flavour_having<invariants::item>([&](const auto& id, const auto& flavour) {
			if (::is_backpack_like(flavour)) {
				const auto price_opt = ::find_price_of(ctx.cosm, item_flavour_id(id));
				if (price_opt.has_value() && *price_opt <= available_money && *price_opt < cheapest_price) {
					cheapest_price = *price_opt;
					cheapest_backpack = item_flavour_id(id);
				}
			}
		});

		return cheapest_backpack;
	};

	auto try_buy_defuse_kit = [&](money_type available_money) -> std::optional<item_flavour_id> {
		std::optional<item_flavour_id> cheapest_kit;
		money_type cheapest_price = std::numeric_limits<money_type>::max();

		ctx.cosm.for_each_flavour_having<invariants::tool>([&](const auto& id, const auto& flavour) {
			const auto& tool = flavour.template get<invariants::tool>();
			if (tool.defusing_speed_mult > 1.f) {
				const auto price_opt = ::find_price_of(ctx.cosm, item_flavour_id(id));
				if (price_opt.has_value() && *price_opt <= available_money && *price_opt < cheapest_price) {
					cheapest_price = *price_opt;
					cheapest_kit = item_flavour_id(id);
				}
			}
		});

		return cheapest_kit;
	};

	auto try_buy_gun = [&](const std::vector<item_flavour_id>& owned_guns, money_type available_money) -> std::optional<item_flavour_id> {
		std::vector<item_flavour_id> affordable_pistols;
		std::vector<item_flavour_id> affordable_non_pistols;

		ctx.cosm.for_each_flavour_having<invariants::gun>([&](const auto& id, const auto& flavour) {
			if (!factions_compatible(ctx.character_handle, id)) {
				return;
			}

			if (flavour.template get<invariants::gun>().bots_ban) {
				return;
			}

			const auto price = *::find_price_of(ctx.cosm, item_flavour_id(id));

			if (price <= available_money) {
				const auto buy_type = flavour.template get<invariants::gun>().buy_type;

				if (buy_type == buy_menu_type::PISTOLS) {
					affordable_pistols.push_back(item_flavour_id(id));
				}
				else {
					affordable_non_pistols.push_back(item_flavour_id(id));
				}
			}
		});

		const auto& weapons_to_choose_from =
			!affordable_non_pistols.empty() ? affordable_non_pistols : affordable_pistols;

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

	auto try_buy_armor = [&](money_type available_money) -> std::optional<item_flavour_id> {
		std::vector<item_flavour_id> affordable_armor;

		ctx.cosm.for_each_flavour_having<invariants::tool>([&](const auto& id, const auto& flavour) {
			if (::is_armor_like(flavour)) {
				const auto price = *::find_price_of(ctx.cosm, item_flavour_id(id));

				if (price <= available_money) {
					affordable_armor.push_back(item_flavour_id(id));
				}
			}
		});

		if (affordable_armor.empty()) {
			return std::nullopt;
		}

		const auto random_index = stable_rng.randval(0u, static_cast<unsigned>(affordable_armor.size() - 1));
		return affordable_armor[random_index];
	};

	const auto [weapon_count, has_only_pistols, owned_guns, has_backpack, has_defuse_kit] = get_owned_inventory();

	/*
		When buying a non-pistol, always buy a backpack first.
	*/
	if (!has_backpack && (weapon_count <= 1 || has_only_pistols)) {
		/*
			Check if we want to buy a non-pistol and if backpack is affordable.
		*/
		if (const auto backpack = try_buy_backpack(money)) {
			const auto backpack_price = *::find_price_of(ctx.cosm, *backpack);
			const auto remaining_after_backpack = money - backpack_price;

			/*
				Check if after buying backpack we can still afford a non-pistol.
			*/
			bool can_afford_non_pistol_after = false;
			ctx.cosm.for_each_flavour_having<invariants::gun>([&](const auto& id, const auto& flavour) {
				if (!factions_compatible(ctx.character_handle, id)) {
					return;
				}
				if (flavour.template get<invariants::gun>().bots_ban) {
					return;
				}
				const auto buy_type = flavour.template get<invariants::gun>().buy_type;
				if (buy_type != buy_menu_type::PISTOLS) {
					const auto price = *::find_price_of(ctx.cosm, item_flavour_id(id));
					if (price <= remaining_after_backpack) {
						can_afford_non_pistol_after = true;
					}
				}
			});

			if (can_afford_non_pistol_after) {
				/*
					Countdown still delays the actual purchase.
				*/
				if (ctx.ai_state.purchase_decision_countdown > 0.0f) {
					return std::nullopt;
				}

				return backpack;
			}
		}
	}

	/*
		Buy gun if needed.
	*/
	if (weapon_count <= 1 || has_only_pistols) {
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
		Buy armor if nothing else.
	*/
	if (const auto armor = try_buy_armor(money)) {
		/*
			Countdown still delays the actual purchase.
		*/
		if (ctx.ai_state.purchase_decision_countdown > 0.0f) {
			return std::nullopt;
		}

		return armor;
	}

	/*
		If we have >=10000 money and don't need to buy anything else, buy defuse kit.
		
		The 10000 threshold ensures bots prioritize essential equipment first.
		Defuse kits are useful but not critical - only buy when we have excess money.
	*/
	constexpr money_type DEFUSE_KIT_THRESHOLD = 10000;
	if (money >= DEFUSE_KIT_THRESHOLD && !has_defuse_kit) {
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

	/*
		Nothing more to buy - set flag so we don't keep checking.
		This can be set even during countdown if we determine there's nothing to purchase.
	*/
	ctx.ai_state.already_nothing_more_to_buy = true;

	return std::nullopt;
}
