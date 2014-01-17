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
	resources::vertex_triangle* last_bound_buffer_location;

	friend class camera_system;
public:
	void call_triangles();
	void push_triangle(const resources::vertex_triangle&);
	void clear_triangles();

	int get_triangle_count();
	resources::vertex_triangle& get_triangle(int i);

	void draw_debug_info();

	struct debug_line {
		debug_line(augmentations::vec2<> a, augmentations::vec2<> b, augmentations::graphics::pixel_32 col = augmentations::graphics::pixel_32(255, 255, 255, 255)) : col(col), a(a), b(b) {}

		augmentations::graphics::pixel_32 col;
		augmentations::vec2<> a, b;
	};
	std::vector<debug_line> lines;
	std::vector<debug_line> manually_cleared_lines;
	std::vector<debug_line> non_cleared_lines;

	void push_line(debug_line l) {
		lines.push_back(l);
	}
	
	void push_non_cleared_line(debug_line l) {
		non_cleared_lines.push_back(l);
	}

	void clear_non_cleared_lines() {
		non_cleared_lines.clear();
	}

	void cleanup();

	fbo scene_fbo, postprocess_fbo;

	float visibility_expansion;
	float max_visibility_expansion_distance;
	int debug_drawing;

	int draw_visibility;

	int draw_steering_forces;
	int draw_substeering_forces;
	int draw_velocities;

	int draw_avoidance_info;
	int draw_wandering_info;

	int draw_weapon_info;

	window::glwindow& output_window;
	render_system(window::glwindow& output_window);

	void process_entities(world&) override;

	void generate_triangles(rects::xywh visible_area, components::transform::state, int mask);
	void default_render(rects::xywh visible_area);
};