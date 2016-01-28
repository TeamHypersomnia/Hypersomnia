#pragma once
#include "window_framework/window.h"
#include "graphics/fbo.h"

#include "entity_system/processing_system.h"

#include "../components/transform_component.h"
#include "../components/render_component.h"

#include "graphics/vertex.h"
#include "texture_baker/texture_baker.h"

#include "../globals/layers.h"

#include <Box2D/Collision/b2DynamicTree.h>

#include <vector>

using namespace augs;

namespace shared {
	struct state_for_drawing_camera;
}

class render_system : public event_only_system {
	std::vector<std::vector<entity_id>> layers;
	std::vector<entity_id> visible_entities;

	b2DynamicTree non_physical_objects_tree;
	unsigned current_step = 0;
public:
	render_system(world& parent_world);

	void add_entities_to_rendering_tree();
	void remove_entities_from_rendering_tree();

	void set_visibility_persistence(entity_id, bool);

	void determine_visible_entities_from_every_camera();

	void set_current_transforms_as_previous_for_interpolation();
	void calculate_and_set_interpolated_transforms();
	void restore_actual_transforms();

	void generate_layers_from_visible_entities(int mask);
	void draw_layer(shared::state_for_drawing_camera in, int layer);
	void draw_all_visible_entities(shared::state_for_drawing_camera in, int mask);

	bool enable_interpolation = true;
	std::vector<entity_id> always_visible_entities;
	std::vector<render_layer> layers_with_custom_drawing_order;
};