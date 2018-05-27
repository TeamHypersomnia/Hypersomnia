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

namespace invariants {
	struct torso {
		// GEN INTROSPECTOR struct invariants::torso
		augs::enum_array<stance_animations, item_holding_stance> stances;
		stance_animations akimbo;

		assets::legs_animation_id forward_legs;
		assets::legs_animation_id strafe_legs;

		unsigned min_strafe_facing = 50;
		unsigned max_strafe_facing = 130;
		// END GEN INTROSPECTOR

		auto calc_leg_anim(
			const vec2 velocity,
			const real32 face_degrees
		) const {
			const auto facing = velocity.degrees_between(vec2::from_degrees(face_degrees));

			if (facing >= min_strafe_facing && facing <= max_strafe_facing) {
				return strafe_legs;
			}
			else {
				return forward_legs;
			}
		}

		template <class C>
		const stance_animations& calc_stance(
			const C& cosm,
			const augs::constant_size_vector<entity_id, 2>& wielded_items
		) const {
			const auto n = wielded_items.size();

			if (n == 0) {
				return stances[item_holding_stance::BARE_LIKE];
			}

			if (n == 2) {
				return akimbo;
			}

			const auto w = cosm[wielded_items[0]];
			return stances[w.template get<invariants::item>().holding_stance];
		}
	};
}

namespace components {

};

template <class T>
auto& calc_stance(const T& typed_handle) {
	const auto& cosm = typed_handle.get_cosmos();
	return typed_handle.template get<invariants::torso>().calc_stance(cosm, typed_handle.get_wielded_items());
}
