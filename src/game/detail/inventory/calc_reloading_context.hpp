#pragma once
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#define LOG_RELOADING 0

#if LOG_RELOADING
#include "augs/log.h"
#endif

template <class... Args>
void RLD_LOG(Args&&... args) {
#if LOG_RELOADING
	LOG(std::forward<Args>(args)...);
#else
	((void)args, ...);
#endif
}

#if LOG_RELOADING
#define RLD_LOG_NVPS LOG_NVPS
#else
#define RLD_LOG_NVPS RLD_LOG
#endif

template <class E, class I>
std::optional<reloading_context> calc_reloading_context_for(const E& capability, const I& candidate_weapon) {
	auto& cosm = capability.get_cosmos();

	if (const auto mag_slot = candidate_weapon[slot_function::GUN_CHAMBER_MAGAZINE]) {
		/* That one is easy, find literally anything that fits */

		RLD_LOG("Found item with chamber mag.");

		entity_id found_ammo;

		auto try_item = [&](const auto& candidate_item) {
			if (mag_slot.can_contain(candidate_item)) {
				found_ammo = candidate_item;

				return recursive_callback_result::ABORT;
			}

			return recursive_callback_result::CONTINUE_AND_RECURSE;
		};

		/* 
			Let hands have precedence so that they break ties 
			once we decide on the charge item.

			This fixes the rocket launcher reloading going haywire.
		*/

		const auto wielded_items = capability.get_wielded_items();

		for (const auto& w : wielded_items) {
			if (try_item(w) == recursive_callback_result::ABORT) {
				break;
			}
		}

		if (!found_ammo.is_set()) {
			const auto traversed_slots = slot_flags {
				slot_function::PERSONAL_DEPOSIT,
				slot_function::ITEM_DEPOSIT,
				slot_function::SHOULDER,
				slot_function::BACK
			};

			capability.for_each_contained_item_recursive(try_item, traversed_slots);
		}

		if (found_ammo.is_set()) {
			RLD_LOG_NVPS(cosm[found_ammo], cosm[found_ammo].get_current_slot());

			reloading_context ctx;

			ctx.concerned_slot = mag_slot;
			ctx.new_ammo_source = found_ammo;
			ctx.old_ammo_source = entity_id::dead();

			return ctx;
		}

		RLD_LOG("Could not find a charge fitting for the chamber.");
	}

	if (const auto mag_slot = candidate_weapon[slot_function::GUN_DETACHABLE_MAGAZINE]) {
		RLD_LOG("Found item with mag.");

		entity_id best_mag;
		entity_id current_mag_id;

		int best_num_charges = -1;
		int current_num_charges = -1;

		auto is_better = [&](const auto& charges, const auto& candidate) {
			if (charges == best_num_charges) {
				/* Break ties with creation time */
				return candidate.get_id().raw.indirection_index < best_mag.raw.indirection_index;
			}

			return charges > best_num_charges;
		};

		auto try_mag = [&](const auto& candidate_mag) {
			if (mag_slot->is_category_compatible_with(candidate_mag)) {
				const auto candidate_charges = count_charges_in_deposit(candidate_mag);

				if (is_better(candidate_charges, candidate_mag)) {
					best_num_charges = candidate_charges;
					best_mag = candidate_mag;
				}

				return true;
			}

			return false;
		};

		if (const auto current_mag = mag_slot.get_item_if_any()) {
			current_mag_id = current_mag.get_id();
			try_mag(current_mag);
			current_num_charges = best_num_charges;
		}

		const auto traversed_slots = slot_flags {
			slot_function::BACK,
			slot_function::SHOULDER,
			slot_function::PRIMARY_HAND,
			slot_function::SECONDARY_HAND,
			slot_function::ITEM_DEPOSIT,
			slot_function::PERSONAL_DEPOSIT
		};

		capability.for_each_contained_item_recursive(
			[&](const auto& candidate_item) {
				if (try_mag(candidate_item)) {
					return recursive_callback_result::CONTINUE_DONT_RECURSE;
				}

				return recursive_callback_result::CONTINUE_AND_RECURSE;
			},
			traversed_slots
		);

		if (best_num_charges > current_num_charges && best_mag != current_mag_id) {
			RLD_LOG("Found best: has %x charges.", best_num_charges);

			reloading_context ctx;

			ctx.concerned_slot = mag_slot;
			ctx.new_ammo_source = best_mag;
			ctx.old_ammo_source = current_mag_id;

			return ctx;
		}

		RLD_LOG("Best is not good enough: %x", best_num_charges);
	}

	return std::nullopt;
}

template <class E>
auto calc_reloading_context(const E& capability) {
	const auto& cosm = capability.get_cosmos();
	const auto items = capability.get_wielded_items();

	/* 
		First, find reloadable weapon. 
		We prioritize the primary hand - the order of items returned by get_wielded_items makes it so.
	*/

	for (const auto& i : items) {
		if (const auto new_ctx = calc_reloading_context_for(capability, cosm[i])) {
			return *new_ctx;
		}
	}

	RLD_LOG("No viable context found.");

	return reloading_context();
}
