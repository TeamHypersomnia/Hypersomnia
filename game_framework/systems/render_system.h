#pragma once
#include "window_framework/window.h"
#include "graphics/fbo.h"

#include "entity_system/processing_system.h"

#include "../components/transform_component.h"
#include "../components/render_component.h"

#include "graphics/vertex.h"
#include "texture_baker/texture_baker.h"

#include <Box2D/Collision/b2DynamicTree.h>

using namespace augs;

namespace shared {
	class drawing_state;
}

class render_system : public event_only_system {
public:
	using event_only_system::event_only_system;

	b2DynamicTree non_physical_objects_tree;

	void add_entities_to_rendering_tree();
	void remove_entities_from_rendering_tree();

	std::vector<std::vector<entity_id>> layers;
	
	std::vector<entity_id> always_visible_entities;
	std::vector<entity_id> visible_entities;

	void set_visibility_persistence(entity_id, bool);

	bool enable_interpolation = true;

	void determine_visible_entities_from_camera_states();

	void generate_layers(shared::drawing_state& in, int mask);
	void draw_layer(shared::drawing_state& in, int layer);
	void generate_and_draw_all_layers(shared::drawing_state& in, int mask);

	void set_current_transforms_as_previous_for_interpolation();
	void calculate_and_set_interpolated_transforms();
	void restore_actual_transforms();
};