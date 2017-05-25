#pragma once
#include "game/transcendental/component_synchronizer.h"
#include "game/enums/tree_of_npo_type.h"
#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "augs/padding_byte.h"

/* NPO stands for "non-physical objects" */

namespace components {
	struct tree_of_npo_node : synchronizable_component {		
		// GEN INTROSPECTOR struct components::tree_of_npo_node
		bool always_visible = false;
		bool activated = true;
		tree_of_npo_type type = tree_of_npo_type::RENDERABLES;

		pad_bytes<1> pad;

		ltrb aabb;
		// END GEN INTROSPECTOR

		static tree_of_npo_node create_default_for(
			const logic_step step,
			const_entity_handle
		);
	};
}

template<bool is_const>
class basic_tree_of_npo_node_synchronizer : public component_synchronizer_base<is_const, components::tree_of_npo_node> {
protected:
public:
	using component_synchronizer_base<is_const, components::tree_of_npo_node>::component_synchronizer_base;

	bool is_activated() const;
};

template<>
class component_synchronizer<false, components::tree_of_npo_node> : public basic_tree_of_npo_node_synchronizer<false> {
	void reinference() const;
public:
	using basic_tree_of_npo_node_synchronizer<false>::basic_tree_of_npo_node_synchronizer;

	void update_proxy(const logic_step) const;
	void set_activated(const bool) const;
};

template<>
class component_synchronizer<true, components::tree_of_npo_node> : public basic_tree_of_npo_node_synchronizer<true> {
public:
	using basic_tree_of_npo_node_synchronizer<true>::basic_tree_of_npo_node_synchronizer;
};