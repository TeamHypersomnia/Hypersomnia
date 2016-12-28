#pragma once
#include <unordered_map>
#include <vector>

#include "augs/graphics/vertex.h"
#include "augs/window_framework/event.h"
#include "gui_event.h"

class rect_world;

namespace augs {
	namespace gui {
		struct draw_info {
			vertex_triangle_buffer& v;
			draw_info(vertex_triangle_buffer&);
		};

		struct event_traversal_flags {
			const augs::window::event::change change;
			
			bool was_hovered_rect_visited = false;
			bool mouse_fetched = false;
			bool scroll_fetched = false;
			event_traversal_flags(const augs::window::event::change);
		};

		struct event_info {
			gui_event msg;

			int scroll_amount;
			vec2i total_dragged_amount;

			event_info(const gui_event, const int scroll_amount = 0, vec2i total_dragged_amount = vec2i());
			operator gui_event() const;
		};

		template <class id_type>
		struct gui_entropy {
			std::unordered_map<id_type, std::vector<event_info>> entries;

			std::vector<event_info> get_events_for(const id_type& id) const {
				const auto it = entries.find(id);

				if (it == entries.end()) {
					return{};
				}
				else {
					return (*it).second;
				}
			}

			void post_event(const id_type& id, const event_info& ev) {
				entries[id].push_back(ev);
			}
		};
	}
}
