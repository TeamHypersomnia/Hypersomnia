#pragma once
#include "game/assets/ids/asset_ids.h"
#include "game/enums/item_holding_stance.h"

#include "augs/misc/enum/enum_array.h"

struct stance_animations {
	// GEN INTROSPECTOR struct stance_animations
	assets::torso_animation_id carry;
	assets::torso_animation_id shoot;
	// END GEN INTROSPECTORS
};

template <class C>
auto calc_stance_id(
	const C& cosm,
	const augs::constant_size_vector<entity_id, 2>& wielded_items
) {
	const auto n = wielded_items.size();

	if (n == 0) {
		return item_holding_stance::BARE_LIKE;
	}

	if (n == 2) {
		return item_holding_stance::AKIMBO;
	}

	const auto w = cosm[wielded_items[0]];
	return w.template get<invariants::item>().holding_stance;
}

namespace invariants {
	struct torso {
		// GEN INTROSPECTOR struct invariants::torso
		augs::enum_array<stance_animations, item_holding_stance> stances;

		assets::legs_animation_id forward_legs;
		assets::legs_animation_id strafe_legs;

		unsigned min_strafe_facing = 50;
		unsigned max_strafe_facing = 130;

		float strafe_face_interp_mult = 0.f;
		// END GEN INTROSPECTOR

		auto calc_leg_anim(
			const vec2 velocity,
			const real32 face_degrees
		) const; 

		template <class... Args>
		const auto& calc_stance(Args&&... args) const {
			return stances[::calc_stance_id(std::forward<Args>(args)...)];
		}
	};
}

namespace components {

}
