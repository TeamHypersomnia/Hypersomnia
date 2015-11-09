#pragma once
#include "window_framework/window.h"
#include "graphics/fbo.h"

#include "entity_system/processing_system.h"

#include "../components/transform_component.h"
#include "../components/render_component.h"

#include "graphics/vertex.h"
#include "../resources/render_info.h"
#include "texture_baker/texture_baker.h"

using namespace augs;


class render_system : public processing_system_templated<components::transform, components::render> {
	augs::vertex_triangle* last_bound_buffer_location;

	friend class camera_system;
public:
	augs::vertex_triangle_buffer triangles;

	GLuint position_buffer, texcoord_buffer, color_buffer;
	GLuint triangle_buffer;

	enum VERTEX_ATTRIBUTES {
		POSITION,
		TEXCOORD,
		COLOR
	};

	void call_triangles();
	void push_triangle(const augs::vertex_triangle&);
	void clear_triangles();

	int get_triangle_count();
	augs::vertex_triangle& get_triangle(int i);

	void fullscreen_quad();
	void draw_debug_info(vec2 visible_area, components::transform, augs::texture* tex);

	struct debug_line {
		debug_line(vec2 a, vec2 b, augs::pixel_32 col = augs::pixel_32(255, 255, 255, 255)) : col(col), a(a), b(b) {}

		augs::pixel_32 col;
		vec2 a, b;
	};
	std::vector<debug_line> lines;
	std::vector<debug_line> lines_channels[20];
	std::vector<debug_line> manually_cleared_lines;
	std::vector<debug_line> non_cleared_lines;

	void push_line(debug_line l) {
		lines.push_back(l);
	}

	void push_line_channel(debug_line l, int i) {
		lines_channels[i].push_back(l);
	}

	void clear_channel(int i) {
		lines_channels[i].clear();
	}

	void push_non_cleared_line(debug_line l) {
		non_cleared_lines.push_back(l);
	}

	void clear_non_cleared_lines() {
		non_cleared_lines.clear();
	}

	void cleanup();

	augs::graphics::fbo scene_fbo, postprocess_fbo;

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
	
	render_system& operator=(const render_system&) {
		assert(0);
		return *this;
	}

	render_system(window::glwindow& output_window);

	void process_entities(world&);


	std::vector<std::vector<entity_id>> layers;

	void generate_layers(int mask);
	void draw_layer(resources::renderable::draw_input& in, int layer);

	void generate_triangles(resources::renderable::draw_input&, int mask);
	void default_render(vec2 visible_area);
};