#pragma once
#include "window_framework/window.h"

#include "entity_system/processing_system.h"

#include "../components/transform_component.h"
#include "../components/render_component.h"

#include "../resources/vertex.h"
#include "../resources/fbo.h"

using namespace augmentations;
using namespace entity_system;

class render_system : public processing_system_templated<components::transform, components::render> {
	resources::buffer triangles;

	friend class camera_system;
public:
	struct debug_line {
		debug_line(augmentations::vec2<> a, augmentations::vec2<> b, augmentations::graphics::pixel_32 col = augmentations::graphics::pixel_32(255, 255, 255, 255)) : col(col), a(a), b(b) {}

		augmentations::graphics::pixel_32 col;
		augmentations::vec2<> a, b;
	};
	std::vector<debug_line> lines;
	std::vector<debug_line> manually_cleared_lines;

	fbo scene_fbo, postprocess_fbo;

	float visibility_expansion;
	float max_visibility_expansion_distance;
	int draw_visibility;

	int draw_steering_forces;
	int draw_substeering_forces;
	int draw_velocities;

	int draw_avoidance_info;

	window::glwindow& output_window;
	render_system(window::glwindow& output_window);

	void process_entities(world&) override;

	void draw(rects::xywh visible_area, components::transform::state, unsigned mask);
	void render(rects::xywh visible_area);
};