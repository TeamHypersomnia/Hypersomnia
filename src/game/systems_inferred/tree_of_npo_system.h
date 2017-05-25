#pragma once
#include <vector>
#include <Box2D/Collision/b2DynamicTree.h>

#include "augs/misc/enum_array.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/components/tree_of_npo_node_component.h"
#include "game/detail/camera_cone.h"

class viewing_step;
class physics_system;

/* NPO stands for "non-physical objects" */

class tree_of_npo_system {
	friend class cosmos;
	
	friend class component_synchronizer<false, components::tree_of_npo_node>;

	struct tree {
		std::vector<unversioned_entity_id> always_visible;
		b2DynamicTree nodes;
	};

	augs::enum_array<tree, tree_of_npo_type> trees;

	struct cache {
		bool constructed = false;
		tree_of_npo_type type = tree_of_npo_type::COUNT;
		int tree_proxy_id = -1;

		bool is_constructed() const;
	};

	std::vector<cache> per_entity_cache;

	void reserve_caches_for_entities(const size_t n);
	void create_inferred_state_for(const const_entity_handle);
	void destroy_inferred_state_of(const const_entity_handle);

	tree& get_tree(const cache&);
	cache& get_cache(const unversioned_entity_id);

public:
	void determine_visible_entities_from_camera(
		std::vector<unversioned_entity_id>& into,
		const camera_cone,
		tree_of_npo_type = tree_of_npo_type::RENDERABLES
	) const;
};