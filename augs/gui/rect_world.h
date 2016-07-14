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
#include "misc/object_pool_handlizer.h"

#include "game/assets/font_id.h"

#include "clipboard.h"
#include "middlescrolling.h"

namespace augs {
	namespace gui {
		class rect_world : object_pool_handlizer<rect_world> {
			float delta_ms = 1000 / 60.f;

		public:
			static clipboard global_clipboard;

			rect_pool rects;

			rect_id rect_in_focus;

			middlescrolling middlescroll;
			
			window::event::state state;

			bool was_hovered_rect_visited = false;
			bool held_rect_is_dragged = false;
			rect_id rect_hovered;
			rect_id rect_held_by_lmb;
			rect_id rect_held_by_rmb;
			
			vec2i ldrag_relative_anchor;
			vec2i last_ldown_position;
			vec2i current_drag_amount;

			rect_id root;

			rect_world();

			void set_delta_milliseconds(float);
			float delta_milliseconds();

			void set_focus(rect_id, std::function<void(rect_handle, rect::event_behaviour)> behaviour);
			rect_id get_rect_in_focus() const;

			void consume_raw_input_and_generate_gui_events(window::event::state, rect::event_behaviour);
			void perform_logic_step(fixed_delta, rect::logic_behaviour, rect::content_size_behaviour);
			vertex_triangle_buffer draw_triangles(rect::draw_behaviour) const;
		};

		void paste_clipboard_formatted(text::fstr& out, text::formatted_char = text::formatted_char());
	}
}

