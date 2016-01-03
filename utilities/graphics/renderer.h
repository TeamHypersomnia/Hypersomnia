#pragma once
#include <vector>
#include "math/vec2.h"
#include "graphics/pixel.h"
#include "graphics/vertex.h"

#include "game_framework/components/transform_component.h"
#include "utilities/entity_system/entity_id.h"

#include "game_framework/assets/texture.h"

namespace augs {
	class renderer {
	public:
		static renderer& get_current();
		
		void initialize();

		enum VERTEX_ATTRIBUTES {
			POSITION,
			TEXCOORD,
			COLOR
		};

		struct debug_line {
			debug_line(vec2 a, vec2 b, augs::pixel_32 col = augs::pixel_32(255, 255, 255, 255)) : col(col), a(a), b(b) {}

			augs::pixel_32 col;
			vec2 a, b;
		};

		augs::vertex_triangle_buffer triangles;

		unsigned int position_buffer, texcoord_buffer, color_buffer;
		unsigned int triangle_buffer;

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

		void fullscreen_quad();
		void draw_debug_info(vec2 visible_area, components::transform, assets::texture_id tex, std::vector<entity_id> target_entities);

		void clear();
		void call_triangles();
		void push_triangle(const augs::vertex_triangle&);
		void clear_triangles();

		void default_render(vec2 visible_area);

		int get_triangle_count();
		augs::vertex_triangle& get_triangle(int i);

		void clear_geometry();

		float visibility_expansion = 1.0f;
		float max_visibility_expansion_distance = 1000.0f;
		int debug_drawing = 0;

		int draw_visibility = 0;

		int draw_steering_forces = 0;
		int draw_substeering_forces = 0;
		int draw_velocities = 0;

		int draw_avoidance_info = 0;
		int draw_wandering_info = 0;

		int draw_weapon_info = 0;
	};
}