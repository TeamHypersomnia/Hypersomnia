#pragma once
#include <Box2D/Collision/b2DynamicTree.h>
#include "game/entity_id.h"

class dynamic_tree_system {
	std::vector<entity_id> visible_entities;

	b2DynamicTree non_physical_objects_tree;

public:

	void add_entities_to_rendering_tree();
	void remove_entities_from_rendering_tree();

	void set_visibility_persistence(entity_id, bool);

	void determine_visible_entities_from_every_camera();

	std::vector<entity_id> always_visible_entities;
	const std::vector<entity_id>& get_all_visible_entities() const;
};