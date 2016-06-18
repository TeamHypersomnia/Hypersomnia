#pragma once
#include "window_framework/window.h"
#include "graphics/fbo.h"

#include "game/components/transform_component.h"
#include "game/components/render_component.h"

#include "graphics/vertex.h"
#include "texture_baker/texture_baker.h"

#include "game/globals/render_layer.h"

#include <vector>

using namespace augs;

namespace shared {
	struct state_for_drawing_camera;
}

class render_system {
	std::vector<std::vector<entity_id>> layers;
	unsigned current_step = 0;
	int current_visibility_index = 0;
public:
	static void standard_draw_entity(entity_id, shared::state_for_drawing_camera in, bool only_border_highlights = false, int visibility_index = -1);

	render_system();

	void set_current_transforms_as_previous_for_interpolation();
	void calculate_and_set_interpolated_transforms();
	void restore_actual_transforms();

	void generate_layers_from_visible_entities(int mask);
	void draw_layer(shared::state_for_drawing_camera in, int layer, bool only_border_highlights = false);
	void draw_all_visible_entities(shared::state_for_drawing_camera in, int mask);

	bool enable_interpolation = true;
	std::vector<render_layer> layers_whose_order_determines_friction;
};