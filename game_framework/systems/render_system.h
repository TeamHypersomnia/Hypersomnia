#pragma once
#include "window_framework/window.h"
#include "graphics/fbo.h"

#include "entity_system/processing_system.h"

#include "../components/transform_component.h"
#include "../components/render_component.h"

#include "graphics/vertex.h"
#include "texture_baker/texture_baker.h"

using namespace augs;

namespace shared {
	class drawing_state;
}

class render_system : public processing_system_templated<components::transform, components::render> {
public:
	using processing_system_templated::processing_system_templated;

	std::vector<std::vector<entity_id>> layers;

	void generate_layers(int mask);
	void draw_layer(shared::drawing_state& in, int layer);
	void generate_and_draw_all_layers(shared::drawing_state& in, int mask);

	void set_current_transforms_as_previous_for_interpolation();
	void calculate_and_set_interpolated_transforms();
	void restore_actual_transforms();
};