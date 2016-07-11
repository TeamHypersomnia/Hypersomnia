#pragma once
#include <vector>
#include <functional>
#include "math/vec2.h"
#include "window_framework/event.h"
#include "misc/timer.h"
#include "graphics/pixel.h"
#include "texture_baker/texture_baker.h"
#include "texture_baker/font.h"
#include "rect.h"

#include "misc/object_pool.h"

#include "game/assets/font_id.h"

#include "clipboard.h"

namespace augs {
	namespace gui {
		class gui_world {
			float delta_ms = 1000 / 60.f;

		public:
			struct middlescroll_data {
				material mat;
				rects::wh<float> size = rects::wh<float>(25, 25);
				vec2i pos;
				rect_id subject;
				float speed_mult = 1.f;
			};

			static clipboard global_clipboard;

			rect_pool rects;

			rect_pool& get_pool();
			const rect_pool& get_pool() const;

			rect_id rect_in_focus;

			middlescroll_data middlescroll;

			augs::window::event::state state;

			bool was_hovered_rect_visited = false;
			bool held_rect_is_dragged = false;
			rect_id rect_hovered;
			rect_id rect_held_by_lmb;
			rect_id rect_held_by_rmb;
			
			vec2i ldrag_relative_anchor;
			vec2i last_ldown_position;
			vec2i current_drag_amount;

			rect_id root;

			gui_world();

			void set_delta_milliseconds(float);
			float delta_milliseconds();

			void set_focus(rect_id, std::function<void(rect_handle, rect::event_behaviour)> behaviour);
			rect_id get_rect_in_focus() const;

			void consume_raw_input_and_generate_gui_events(rect::event_behaviour, augs::window::event::state);
			void perform_logic_step(rect::logic_behaviour);
			vertex_triangle_buffer draw_triangles(rect::draw_behaviour) const;
		};

		void paste_clipboard_formatted(text::fstr& out, text::formatted_char = text::formatted_char());
	}
}

