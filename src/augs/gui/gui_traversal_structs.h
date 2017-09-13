#pragma once
#include <unordered_map>
#include <vector>

#include "augs/graphics/vertex.h"
#include "augs/window_framework/event.h"
#include "gui_event.h"

namespace augs {
	namespace gui {
		struct raw_input_traversal {
			const augs::event::change change;
			bool was_hovered_rect_visited = false;

			raw_input_traversal(const augs::event::change);
		};

		struct event_info {
			gui_event msg;

			int scroll_amount;
			vec2i total_dragged_amount;

			bool is_ldown_or_double_or_triple() const;

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
