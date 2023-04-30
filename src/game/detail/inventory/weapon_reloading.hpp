#pragma once
#include "game/detail/inventory/weapon_reloading.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/inventory/inventory_utils.h"
#include "game/detail/gun/gun_cooldowns.h"

inline bool reloading_context::significantly_different_from(const reloading_context& b) const {
	return 
		concerned_slot != b.concerned_slot 
		|| new_ammo_source != b.new_ammo_source 
	;
}

inline bool reloading_context::is_chambering() const {
	return !new_ammo_source.is_set() && !old_ammo_source.is_set();
}

enum class reloading_movement_type {
	POCKET_TO_MAG,
	GRIP_TO_MAG
};

struct reloading_movement {
	entity_id weapon;
	entity_id mag;
	reloading_movement_type type;
	real32 progress_ms;
};

template <class E>
bool gun_shot_cooldown(const E& gun_entity) {
	const auto& cosm = gun_entity.get_cosmos();

	if (const auto gun_def = gun_entity.template find<invariants::gun>()) {
		if (const auto gun = gun_entity.template find<components::gun>()) {
			const auto& clk = cosm.get_clock();
			const auto when_transferred = gun_entity.when_last_transferred();

			const bool shot_cooldown_passed = clk.is_ready(gun_def->shot_cooldown_ms, gun->fire_cooldown_object);

			const bool transfer_cooldown_passed = clk.is_ready(
				gun_def->get_transfer_shot_cooldown(), 
				when_transferred
			);

			if (!transfer_cooldown_passed || !shot_cooldown_passed) {
				return true;
			}
		}
	}
	
	return false;
}

template <class E>
bool gun_shot_cooldown_or_chambering_from_chamber_mag(const E& gun_entity) {
	if (gun_shot_cooldown(gun_entity)) {
		return true;
	}
	
	if (gun_entity[slot_function::GUN_CHAMBER].is_empty_slot()) {
		if (const auto mag_chamber = gun_entity[slot_function::GUN_CHAMBER_MAGAZINE]) {
			if (mag_chamber.has_items()) {
				return true;
			}
		}
	}

	return false;
}

template <class E>
bool currently_chambering(const E& gun_entity) {
	if (const auto gun_def = gun_entity.template find<invariants::gun>()) {
		if (const auto gun = gun_entity.template find<components::gun>()) {
			const auto& chambering_progress = gun->chambering_progress_ms;
			const auto chambering_duration = ::calc_current_chambering_duration(gun_entity);

			return chambering_progress > 0.f && augs::is_positive_epsilon(chambering_duration);
		}
	}

	return false;
}

template <class C, class W>
std::optional<reloading_movement> calc_reloading_movement(
	const C& cosm,
	const W& wielded_items
) {
	const auto n = wielded_items.size();

	if (n == 0) {
		return std::nullopt;
	}

	auto is_during_shot_action = [](const auto& gun) {
		return gun_shot_cooldown(gun) || currently_chambering(gun);
	};

	{
		/* Context-full reloading. Will happen most of the time, e.g. when R is pressed or it is initialied automatically. */

		const auto capability = cosm[wielded_items[0]].get_current_slot().get_container();

		if (const auto transfers = capability.template find<components::item_slot_transfers>()) {
			const auto& ctx = transfers->current_reloading_context;

			if (const auto w0 = cosm[ctx.concerned_slot]) {
				real32 progress_ms = 0.f;

				const auto source = cosm[n == 1 ? ctx.old_ammo_source : ctx.new_ammo_source];

				if (source) {
					if (const auto progress = source.find_mounting_progress()) {
						progress_ms = progress->progress_ms;
					}
				}

				const auto container = w0.get_container();

				if (is_during_shot_action(container)) {
					return std::nullopt;
				}

				return reloading_movement {
					container,
					source,
					n == 1 ? reloading_movement_type::GRIP_TO_MAG : reloading_movement_type::POCKET_TO_MAG,
					progress_ms
				};
			}
		}
	}

	/* Context-less reloading. Might happen if we manually initialize the reloading from the GUI. */

	if (n == 1) {
		const auto w0 = cosm[wielded_items[0]];

		if (is_during_shot_action(w0)) {
			return std::nullopt;
		}

		if (const auto mag_slot = w0[slot_function::GUN_DETACHABLE_MAGAZINE]) {
			if (const auto mag = mag_slot.get_item_if_any()) {
				if (const auto progress = mag.find_mounting_progress(); progress && progress->progress_ms > 0.f) {
					return reloading_movement {
						w0, mag, reloading_movement_type::GRIP_TO_MAG, progress->progress_ms
					};
				}
			}
		}
	}

	if (n == 2) {
		/* 
			Note that a mag can also be a wielded item,
			so when we're putting a new mag into the target slot, there are 2 items wielded,
			not 1.
		*/

		for (int i = 0; i < 2; ++i) {
			const auto wi = cosm[wielded_items[i]];

			const auto detachable_mag_slot = wi[slot_function::GUN_DETACHABLE_MAGAZINE];
			const auto chamber_mag_slot = wi[slot_function::GUN_CHAMBER_MAGAZINE];

			if (detachable_mag_slot || chamber_mag_slot) {
				if (const auto new_source = cosm[wielded_items[1 - i]]) {
					if (const auto progress = new_source.find_mounting_progress(); progress && progress->progress_ms > 0.f) {
						if (is_during_shot_action(wi)) {
							return std::nullopt;
						}

						return reloading_movement {
							wi, new_source, reloading_movement_type::POCKET_TO_MAG, progress->progress_ms
						};
					}
				}
			}
		}
	}

	return std::nullopt;
}

template <class E>
auto calc_reloading_movement(
	const E& typed_handle
) {
	return calc_reloading_movement(typed_handle.get_cosmos(), typed_handle.get_wielded_items());
}

template <class C, class W>
bool is_currently_reloading(
	const C& cosm,
	const W& wielded_items
) {
	return ::calc_reloading_movement(cosm, wielded_items).has_value();
}

template <class E>
bool is_currently_reloading(
	const E& typed_handle
) {
	return is_currently_reloading(typed_handle.get_cosmos(), typed_handle.get_wielded_items());
}
