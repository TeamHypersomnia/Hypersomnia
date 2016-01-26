#pragma once
#include <vector>
#include "math/vec2.h"
#include "graphics/pixel.h"
#include "graphics/vertex.h"

#include "game_framework/components/transform_component.h"
#include "utilities/entity_system/entity_id.h"

#include "game_framework/assets/texture.h"
#include "utilities/misc/timer.h"

namespace augs {
	namespace graphics {
		namespace gui {
			class gui_world;
		}
	}

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
			debug_line(vec2 a = vec2(), vec2 b = vec2(), rgba col = rgba(255, 255, 255, 255)) : col(col), a(a), b(b) {}

			rgba col;
			vec2 a, b;
		};

		vertex_triangle_buffer triangles;

		unsigned int position_buffer, texcoord_buffer, color_buffer;
		unsigned int triangle_buffer;

		struct line_channel {
			std::vector<debug_line> lines;

			void draw(vec2 a, vec2 b, rgba = colors::white);
			
			void draw_red(vec2 a, vec2 b);
			void draw_green(vec2 a, vec2 b);
			void draw_blue(vec2 a, vec2 b);
			void draw_yellow(vec2 a, vec2 b);
			void draw_cyan(vec2 a, vec2 b);
		};

		line_channel logic_lines, prev_logic_lines;
		line_channel frame_lines;
		line_channel blink_lines;
		timer line_timer;

		void fullscreen_quad();
		
		void clear_logic_lines();
		void clear_frame_lines();
		void draw_debug_info(vec2 visible_area, components::transform, assets::texture_id tex, std::vector<entity_id> target_entities, double interpolation_ratio);

		void clear();
		void call_triangles();
		void viewport(rects::xywh<int>);
		void push_triangle(const vertex_triangle&);
		void push_triangles_from_gui_world(graphics::gui::gui_world&);

		void clear_triangles();

		void default_render(vec2 visible_area);

		int get_triangle_count();
		vertex_triangle& get_triangle(int i);

		void clear_geometry();

		float visibility_expansion = 1.0f;
		float max_visibility_expansion_distance = 1000.0f;
		int debug_drawing = 1;

		int draw_visibility = 0;

		int draw_steering_forces = 0;
		int draw_substeering_forces = 0;
		int draw_velocities = 0;

		int draw_avoidance_info = 0;
		int draw_wandering_info = 0;

		int draw_weapon_info = 0;
	};
}