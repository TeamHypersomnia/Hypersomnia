#pragma once
#include <vector>
#include "math/vec2.h"
#include "graphics/pixel.h"
#include "graphics/vertex.h"

#include "game/components/transform_component.h"
#include "augs/entity_system/entity_id.h"

#include "game/assets/texture_id.h"
#include "augs/misc/timer.h"

namespace augs {
	namespace gui {
		class gui_world;
	}

	class renderer {
	public:
		static renderer& get_current();
		
		void initialize();

		enum VERTEX_ATTRIBUTES {
			POSITION,
			TEXCOORD,
			COLOR,
			SPECIAL
		};

		struct debug_line {
			debug_line(vec2 a = vec2(), vec2 b = vec2(), rgba col = rgba(255, 255, 255, 255)) : col(col), a(a), b(b) {}

			rgba col;
			vec2 a, b;
		};

		vertex_triangle_buffer triangles;
		vertex_line_buffer lines;
		special_buffer specials;

		unsigned long long triangles_drawn_total = 0;

		unsigned int position_buffer_id, texcoord_buffer_id, color_buffer_id;
		unsigned int triangle_buffer_id;
		unsigned int special_buffer_id;

		struct line_channel {
			std::vector<debug_line> lines;

			void draw(vec2 a, vec2 b, rgba = white);
			
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
		void draw_debug_info(vec2 visible_world_area, components::transform, assets::texture_id tex, std::vector<entity_id> target_entities, double interpolation_ratio);

		void clear_current_fbo();
		void enable_special_vertex_attribute();
		void disable_special_vertex_attribute();
		void call_triangles();
		void call_lines();
		void set_viewport(rects::xywh<int>);
		void push_line(const vertex_line&);
		void push_triangle(const vertex_triangle&);
		void push_triangles_from_gui_world(gui::gui_world&);
		void push_special_vertex_triangle(augs::special, augs::special, augs::special);

		void clear_special_vertex_data();
		void clear_triangles();
		void clear_lines();

		void default_render(vec2 visible_world_area);

		int get_triangle_count();
		vertex_triangle& get_triangle(int i);
		std::vector<vertex_triangle>& get_triangle_buffer();

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