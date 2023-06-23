#pragma once
#include "game/components/torso_component.h"
#include "game/detail/inventory/weapon_reloading.hpp"

struct leg_animation_usage {
	assets::legs_animation_id id;
	bool flip_vertically = false;
	float rotation = 0.f;
};

template <class E>
auto calc_stance_id(
	const E& typed_entity,
	const augs::constant_size_vector<entity_id, 2>& wielded_items,
	const bool consider_weapon_reloading
) {
	const auto& cosm = typed_entity.get_cosmos();
	const auto n = wielded_items.size();

	if (n == 0) {
		return item_holding_stance::FISTS_LIKE;
	}

	auto stance_of = [&](const auto i) {
		const auto w = cosm[wielded_items[i]];
		return w.template get<invariants::item>().holding_stance;
	};

	if (n == 2) {
		if (stance_of(0) == item_holding_stance::BARE_LIKE
			&& stance_of(1) == item_holding_stance::BARE_LIKE
		) {
			return item_holding_stance::BARE_LIKE;
		}

		if (consider_weapon_reloading) {
			/*
				While reloading,
				a magazine might be put into another hand which would naturally initiate AKIMBO.
				However, we don't want to show akimbo during reloading, we want the particular stance of the reloaded weapon.
			*/

			const auto is_reloading = ::is_currently_reloading(typed_entity.get_cosmos(), wielded_items);
			
			if (is_reloading) {
				if (const auto s0 = stance_of(0); s0 != item_holding_stance::BARE_LIKE) {
					return s0;
				}

				if (const auto s1 = stance_of(1); s1 != item_holding_stance::BARE_LIKE) {
					return s1;
				}
			}
		}

		return item_holding_stance::AKIMBO;
	}

	return stance_of(0);
}

namespace invariants {
	template <class... Args>
	const stance_animations& torso::calc_stance(Args&&... args) const {
		return stances[::calc_stance_id(std::forward<Args>(args)...)];
	}

	inline auto torso::calc_leg_anim(
		vec2 legs_dir,
		const real32 face_degrees
	) const {
		leg_animation_usage output;

		legs_dir.normalize();

		const auto face_dir = vec2::from_degrees(face_degrees);
		const auto facing = legs_dir.degrees_between(face_dir);

		if (facing >= min_strafe_facing && facing <= max_strafe_facing) {
			output.id = strafe_legs;

			vec2 perp_face;

			if (face_dir.cross(legs_dir) < 0.f) {
				output.flip_vertically = true;
				perp_face = face_dir.perpendicular_ccw();
			}
			else {
				perp_face = face_dir.perpendicular_cw();
			}

			const auto lerped = legs_dir.lerp(perp_face, strafe_face_interp_mult);;
			legs_dir = lerped;
		}
		else {
			if (facing > 90) {
				legs_dir.neg();
			}

			const auto lerped = legs_dir.lerp(face_dir, strafe_face_interp_mult);;
			legs_dir = lerped;

			output.id = forward_legs;
		}

		output.rotation = legs_dir.degrees();
		return output;
	}
}
