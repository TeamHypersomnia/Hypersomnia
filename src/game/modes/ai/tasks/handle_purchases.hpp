#pragma once
#include <optional>
#include "augs/misc/randomization.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/modes/detail/item_purchase_logic.hpp"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/arena_mode_ai_structs.h"

inline std::optional<item_flavour_id> handle_purchases(
	const ai_character_context& ctx,
	const money_type money,
	const float dt_secs,
	randomization& stable_rng
) {
	if (ctx.ai_state.already_tried_to_buy || money <= 0) {
		return std::nullopt;
	}

	if (ctx.ai_state.purchase_decision_countdown < 0.0f) {
		ctx.ai_state.purchase_decision_countdown = stable_rng.randval(1.0f, 3.0f);
	}

	ctx.ai_state.purchase_decision_countdown -= dt_secs;

	if (ctx.ai_state.purchase_decision_countdown > 0.0f) {
		return std::nullopt;
	}

	ctx.ai_state.already_tried_to_buy = true;

	auto get_owned_guns = [&]() {
		bool has_only_pistols = true;
		int weapon_count = 0;
		std::vector<item_flavour_id> owned_guns;

		ctx.character_handle.for_each_contained_item_recursive(
			[&](const auto& item) {
				if (const auto gun = item.template find<invariants::gun>()) {
					weapon_count++;
					owned_guns.push_back(item.get_flavour_id());

					if (gun->buy_type != buy_menu_type::PISTOLS) {
						has_only_pistols = false;
					}
				}
			}
		);

		return std::tuple{ weapon_count, has_only_pistols, owned_guns };
	};

	auto try_buy_gun = [&](const std::vector<item_flavour_id>& owned_guns) -> std::optional<item_flavour_id> {
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

			if (price <= money) {
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

	auto try_buy_armor = [&]() -> std::optional<item_flavour_id> {
		std::vector<item_flavour_id> affordable_armor;

		ctx.cosm.for_each_flavour_having<invariants::tool>([&](const auto& id, const auto& flavour) {
			if (::is_armor_like(flavour)) {
				const auto price = *::find_price_of(ctx.cosm, item_flavour_id(id));

				if (price <= money) {
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

	const auto [weapon_count, has_only_pistols, owned_guns] = get_owned_guns();

	if (weapon_count <= 1 || has_only_pistols) {
		if (const auto gun = try_buy_gun(owned_guns)) {
			return gun;
		}
	}

	return try_buy_armor();
}
