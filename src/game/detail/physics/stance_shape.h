#pragma once
#include "augs/math/vec2.h"
#include "augs/misc/constant_size_vector.h"
#include "game/assets/image_offsets.h"
#include "game/enums/item_holding_stance.h"
#include "game/components/torso_component.hpp"

/*
	The shape of a character's colliders in a stance: that of the first frame of the stance's carry animation,
	turned by the frame's body_rotation and mirrored when only the secondary hand holds an item -
	the torso sprite is drawn flipped then. A frame without a custom shape makes a box of the character's size.
*/

struct stance_shape {
	bool found_frame = false;
	const image_shape_type* shape = nullptr;

	/*
		The raw, unflipped body_rotation of the frame -
		mirroring the turned shape negates it implicitly, as flip(rotate(r, v)) == rotate(-r, flip(v)).
	*/
	real32 shape_rotation = 0.f;

	real32 box_rotation = 0.f;
};

template <class E>
item_holding_stance calc_collider_stance_id(const E& character) {
	const bool consider_reloading = true;
	return ::calc_stance_id(character, character.get_wielded_items(), consider_reloading);
}

template <class E>
stance_shape find_stance_shape(const E& character, const item_holding_stance stance_id, const bool mirrored) {
	auto result = stance_shape();

	if (const auto torso = character.template find<invariants::torso>()) {
		const auto& logicals = character.get_cosmos().get_logical_assets();

		if (const auto anim = logicals.find(torso->stances[stance_id].carry)) {
			if (anim->frames.size() > 0) {
				const auto& offsets = logicals.get_offsets(anim->frames[0].image_id);

				result.found_frame = true;

				if (!offsets.non_standard_shape.empty()) {
					result.shape = &offsets.non_standard_shape;
					result.shape_rotation = offsets.torso.body_rotation;
				}

				auto considered_offsets = offsets.torso;

				if (mirrored) {
					considered_offsets.flip_vertically();
				}

				considered_offsets.apply_body_rotation();

				result.box_rotation = considered_offsets.back.rotation;
			}
		}
	}

	return result;
}

/*
	The outline of the colliders a character has in a stance, in its own space, in pixels.
*/

using stance_outline = augs::constant_size_vector<vec2, 32>;

template <class E>
stance_outline calc_stance_outline(const E& character, const item_holding_stance stance_id, const bool mirrored) {
	const auto stance = ::find_stance_shape(character, stance_id, mirrored);

	auto result = stance_outline();

	if (stance.shape != nullptr) {
		for (auto v : stance.shape->source_polygon) {
			v.rotate(stance.shape_rotation);

			if (mirrored) {
				v.neg_y();
			}

			result.push_back(v);
		}

		return result;
	}

	const auto half = character.get_logical_size() / 2;

	for (const auto corner : { vec2(-half.x, -half.y), vec2(half.x, -half.y), vec2(half.x, half.y), vec2(-half.x, half.y) }) {
		result.push_back(vec2(corner).rotate(stance.box_rotation));
	}

	return result;
}
