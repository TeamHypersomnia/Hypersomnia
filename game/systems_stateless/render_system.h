#pragma once
#include "game/enums/render_layer.h"

#include <vector>
#include <array>

#include "augs/graphics/vertex.h"
#include "game/transcendental/entity_handle_declaration.h"

using namespace augs;

struct state_for_drawing_camera;

class render_system {
public:
	static bool render_order_compare(const const_entity_handle& a, const const_entity_handle& b);

	void draw_entities(
		augs::vertex_triangle_buffer& output, 
		std::vector<const_entity_handle>, 
		state_for_drawing_camera in, 
		bool only_border_highlights = false) const;


	template<class renderable_type>
	void draw_renderable(
		augs::vertex_triangle_buffer& output,
		const renderable_type& renderable,
		const components::transform& renderable_transform,
		const components::render& render,
		state_for_drawing_camera in_camera,
		bool only_border_highlights = false) const {

		components::transform camera_transform;
		camera_transform = render.absolute_transform ? components::transform() : in_camera.camera_transform;

		typename renderable_type::drawing_input in(output);

		in.camera_transform = camera_transform;
		in.renderable_transform = renderable_transform;
		in.visible_world_area = in_camera.visible_world_area;

		if (only_border_highlights) {
			if (render.draw_border) {
				static vec2i offsets[4] = {
					vec2i(-1, 0), vec2i(1, 0), vec2i(0, 1), vec2i(0, -1)
				};

				in.colorize = render.border_color;

				for (auto& o : offsets) {
					in.renderable_transform.pos = renderable_transform.pos + o;
					renderable.draw(in);
				}
			}
		}
		else {
			renderable.draw(in);
		}
	}

	std::array<std::vector<const_entity_handle>, render_layer::LAYER_COUNT> get_visible_per_layer(const std::vector<const_entity_handle>&) const;
};