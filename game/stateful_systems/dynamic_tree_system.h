#pragma once
#include <Box2D/Collision/b2DynamicTree.h>
#include "game/entity_id.h"
#include "game/entity_handle_declaration.h"
#include <vector>

class variable_step;

class dynamic_tree_system {
	friend class cosmos;

	std::vector<entity_id> always_visible_entities;

	b2DynamicTree non_physical_objects_tree;

	struct cache {
		bool is_constructed = false;
		int tree_proxy_id = -1;

		bool is_constructed() const {
			return is_constructed;
		}
	};

	std::vector<cache> per_entity_cache;

	void reserve_caches_for_entities(size_t n);
	void construct(const_entity_handle);
	void destruct(const_entity_handle);
public:

	std::vector<entity_id> determine_visible_entities_from_camera(const_entity_handle camera) const;
};