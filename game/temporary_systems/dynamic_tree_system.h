#pragma once
#include <Box2D/Collision/b2DynamicTree.h>
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include <vector>
#include "game/detail/state_for_drawing_camera.h"

class viewing_step;
class physics_system;

class dynamic_tree_system {
	friend class cosmos;

	std::vector<unversioned_entity_id> always_visible_entities;

	b2DynamicTree non_physical_objects_tree;

	struct cache {
		bool constructed = false;
		int tree_proxy_id = -1;

		bool is_constructed() const;
	};

	std::vector<cache> per_entity_cache;

	void reserve_caches_for_entities(size_t n);
	void construct(const_entity_handle);
	void destruct(const_entity_handle);
public:

	std::vector<unversioned_entity_id> determine_visible_entities_from_camera(state_for_drawing_camera, const physics_system&) const;
};