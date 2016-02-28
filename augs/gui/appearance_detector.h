#pragma once
#include "rect.h"

namespace augs {
	namespace gui {
		struct appearance_detector {
			enum class appearance {
				unknown,
				released,
				hovered,
				pushed
			};

			appearance current_appearance = appearance::released;

			/* how should rect look like depending on incoming event */
			static appearance map_event_to_appearance_type(rect::gui_event m);

			void update_appearance(rect::gui_event);

		protected:
			bool focus_flag = false;
		};
	}
}