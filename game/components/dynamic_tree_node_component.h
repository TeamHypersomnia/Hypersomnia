#pragma once
#include "game/component_synchronizer.h"
#include "math/vec2.h"
#include "math/rects.h"

namespace components {
	struct dynamic_tree_node {
		bool always_visible = false;
		bool activated = true;

		rects::ltrb<float> aabb;

		static dynamic_tree_node get_default(const_entity_handle);
	};
}

template<bool is_const>
class component_synchronizer<is_const, components::dynamic_tree_node> : public component_synchronizer_base<is_const, components::dynamic_tree_node> {
public:
	using component_synchronizer_base<is_const, components::dynamic_tree_node>::component_synchronizer_base;
};