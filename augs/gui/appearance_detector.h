#pragma once
#include "rect.h"

namespace augs {
	namespace gui {
		struct appearance_detector {
			enum class appearance {
				unknown,
				released,
				pushed
			};

			bool is_hovered = false;
			bool remain_pushed_if_mouse_leaves = true;

			appearance current_appearance = appearance::released;

			void update_appearance(rect::gui_event);

		protected:
			bool focus_flag = false;
		};
	}
}