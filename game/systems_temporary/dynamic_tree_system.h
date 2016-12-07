#pragma once
#include <Box2D/Collision/b2DynamicTree.h>
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include <vector>
#include "game/detail/state_for_drawing_camera.h"
#include "game/components/dynamic_tree_node_component.h"

class viewing_step;
class physics_system;

class dynamic_tree_system {
	friend class cosmos;
	
	friend class component_synchronizer<false, components::dynamic_tree_node>;

	struct tree {
		std::vector<unversioned_entity_id> always_visible;
		b2DynamicTree nodes;
	};

	tree trees[static_cast<size_t>(components::dynamic_tree_node::tree_type::COUNT)];

	struct cache {
		bool constructed = false;
		components::dynamic_tree_node::tree_type type;
		int tree_proxy_id = -1;

		bool is_constructed() const;
	};

	std::vector<cache> per_entity_cache;

	void reserve_caches_for_entities(size_t n);
	void construct(const_entity_handle);
	void destruct(const_entity_handle);

	tree& get_tree(const cache&);
	cache& get_cache(const unversioned_entity_id);

public:

	std::vector<unversioned_entity_id> determine_visible_entities_from_camera(
		const camera_cone, 
		components::dynamic_tree_node::tree_type = components::dynamic_tree_node::tree_type::RENDERABLES) const;
};