#pragma once
#include "augs/math/math.h"
#include "game/components/torso_component.hpp"
#include "game/detail/frame_calculation.h"
#include "game/detail/inventory/direct_attachment_offset.h"

template <class E>
std::optional<transformr> calc_head_transform(const E& typed_handle) {
	// CHUNKS COPIED DIRECTLY FROM RENDERING CODE

	const auto& torso = typed_handle.template get<invariants::torso>();
	const auto& movement = typed_handle.template get<components::movement>();

	const auto& cosm = typed_handle.get_cosmos();
	const auto& logicals = cosm.get_logical_assets();

	const auto& head_def = typed_handle.template get<invariants::head>();

	const auto wielded_items = typed_handle.get_wielded_items();
	const bool consider_weapon_reloading = true;

	const auto stance_id = ::calc_stance_id(typed_handle, wielded_items, consider_weapon_reloading);
	const auto& stance = torso.stances[stance_id];

	auto four_ways = movement.four_ways_animation;

	if (movement.was_walk_effective) {
		four_ways.index = 0;
	}

	if (const auto stance_usage = ::calc_stance_usage(
		typed_handle,
		stance, 
		four_ways,
		wielded_items
	)) {
		const auto stance_offsets = [&stance_usage, &logicals]() {
			const auto stance_image_id = stance_usage.frame->image_id;

			auto result = logicals.get_offsets(stance_image_id).torso;

			if (stance_usage.movement_flip.vertically) {
				result.flip_vertically();
			}

			return result;
		}();

		const auto target_image = 
			stance_usage.flags.test(stance_flag::SHOOTING)
			? head_def.shooting_head_image 
			: head_def.head_image
		;

		const auto& head_offsets = logicals.get_offsets(target_image);

		auto stance_offsets_for_head = stance_offsets;
		auto anchor_for_head = head_offsets.item.head_anchor;

		const bool only_secondary = typed_handle.only_secondary_holds_item();

		if (only_secondary) {
			stance_offsets_for_head.flip_vertically();
			anchor_for_head.flip_vertically();
		}

		const auto& viewing_transform = typed_handle.get_logic_transform();

		const auto target_offset = ::get_anchored_offset(stance_offsets_for_head.head, anchor_for_head);
		const auto target_transform = viewing_transform * target_offset;

		return target_transform;
	}

	return std::nullopt;
}

inline bool headshot_detected_finite_ray(
	const vec2 from,
	const vec2 to,
	const vec2 head_center,
	const real32 head_radius
) {
	const auto result = ::circle_ray_intersection(
		from,
		to,
		head_center,
		head_radius
	);

	return result.hit;
}

inline bool headshot_detected(
	const vec2 missile_center,
	const vec2 missile_direction,
	const vec2 head_center,
	const real32 head_radius
) {
	const auto ray_length = 200.0f;
	const auto end = missile_center + missile_direction * ray_length;

	const auto result = ::circle_ray_intersection(
		missile_center,
		end,
		head_center,
		head_radius
	);

	return result.hit;
}

