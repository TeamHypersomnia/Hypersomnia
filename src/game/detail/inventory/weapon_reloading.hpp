#pragma once
#include "game/detail/inventory/weapon_reloading.h"

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

template <class C, class W>
std::optional<reloading_movement> calc_reloading_movement(
	const C& cosm,
	const W& wielded_items
) {
	const auto n = wielded_items.size();

	if (n == 0) {
		return std::nullopt;
	}

	{
		const auto capability = cosm[wielded_items[0]].get_current_slot().get_container();

		if (const auto transfers = capability.template find<components::item_slot_transfers>()) {
			const auto& ctx = transfers->current_reloading_context;

			if (const auto w0 = cosm[ctx.concerned_slot]) {
				const auto source = cosm[n == 1 ? ctx.old_ammo_source : ctx.new_ammo_source];

				real32 progress_ms = 0.f;

				if (const auto progress = source.find_mounting_progress()) {
					progress_ms = progress->progress_ms;
				}

				return reloading_movement {
					w0.get_container(),
					source,
					n == 1 ? reloading_movement_type::GRIP_TO_MAG : reloading_movement_type::POCKET_TO_MAG,
					progress_ms
				};
			}
		}
	}

	if (n == 1) {
		const auto w0 = cosm[wielded_items[0]];
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
		for (int i = 0; i < 2; ++i) {
			const auto wi = cosm[wielded_items[i]];

			if (const auto mag_slot = wi[slot_function::GUN_DETACHABLE_MAGAZINE]) {
				if (const auto new_source = cosm[wielded_items[1 - i]]) {
					if (const auto progress = new_source.find_mounting_progress(); progress && progress->progress_ms > 0.f) {
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
	return ::calc_reloading_movement(cosm, wielded_items) != std::nullopt;
}

template <class E>
bool is_currently_reloading(
	const E& typed_handle
) {
	return is_currently_reloading(typed_handle.get_cosmos(), typed_handle.get_wielded_items());
}
