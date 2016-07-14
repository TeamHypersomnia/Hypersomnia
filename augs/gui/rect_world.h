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

#include "misc/pool.h"
#include "misc/pool_handlizer.h"

#include "game/assets/font_id.h"

#include "clipboard.h"
#include "middlescrolling.h"

#include "misc/delta.h"

namespace augs {
	namespace gui {
		class rect_world {
		public:
			static clipboard global_clipboard;

			fixed_delta delta;

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

			void set_focus(rect_handle, event_behaviour behaviour);
			rect_id get_rect_in_focus() const;

			void consume_raw_input_and_generate_gui_events(rect_pool&, window::event::state, event_behaviour);
			void perform_logic_step(rect_pool&, event_behaviour, logic_behaviour, content_size_behaviour);
			vertex_triangle_buffer draw_triangles(const rect_pool&, draw_behaviour) const;
		};

		struct draw_info {
			const rect_world& owner;
			vertex_triangle_buffer& v;

			draw_info(const rect_world&, vertex_triangle_buffer&);
		};

		struct raw_event_info {
			rect_world& owner;
			const unsigned msg;

			bool mouse_fetched = false;
			bool scroll_fetched = false;
			raw_event_info(rect_world&, unsigned);
		};

		struct event_info {
			rect_world& owner;
			gui_event msg;

			event_info(rect_world&, gui_event);
			operator gui_event();
			event_info& operator=(gui_event);
		};
	}
}

