#pragma once
#include "game/transcendental/component_synchronizer.h"
#include "augs/math/vec2.h"
#include "augs/math/rects.h"

namespace components {
	struct dynamic_tree_node {
		bool always_visible = false;
		bool activated = true;

		augs::rects::ltrb<float> aabb;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(always_visible),
				CEREAL_NVP(activated),
				
				CEREAL_NVP(aabb)
			);
		}

		static dynamic_tree_node get_default(const_entity_handle);
	};
}

template<bool is_const>
class component_synchronizer<is_const, components::dynamic_tree_node> : public component_synchronizer_base<is_const, components::dynamic_tree_node> {
public:
	using component_synchronizer_base<is_const, components::dynamic_tree_node>::component_synchronizer_base;
};