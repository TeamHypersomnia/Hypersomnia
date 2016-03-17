#include "appearance_detector.h"

namespace augs {
	namespace gui {
		void appearance_detector::update_appearance(rect::gui_event m) {
			
			appearance app = appearance::unknown;

			if (!remain_pushed_if_mouse_leaves) {
				if (m == rect::gui_event::hoverlost
					|| m == rect::gui_event::lup
					|| m == rect::gui_event::loutup
					|| m == rect::gui_event::rup
					|| m == rect::gui_event::routup
					)
					app = appearance::released;
			}
			else
				if (m == rect::gui_event::lup
					|| m == rect::gui_event::loutup
					|| m == rect::gui_event::rup
					|| m == rect::gui_event::routup
					)
					app = appearance::released;

			if (m == rect::gui_event::hover) {
				is_hovered = true;
			}

			if (m == rect::gui_event::lpressed
				|| m == rect::gui_event::ldown
				|| m == rect::gui_event::ldoubleclick
				|| m == rect::gui_event::ltripleclick
				|| m == rect::gui_event::rpressed
				|| m == rect::gui_event::rdown
				|| m == rect::gui_event::rdoubleclick
				)
				app = appearance::pushed;

			if (m == rect::gui_event::focus)
				focus_flag = true;
			if (m == rect::gui_event::blur)
				focus_flag = false;
			if (m == rect::gui_event::hoverlost)
				is_hovered = false;

			if (app != appearance::unknown)
				current_appearance = app;
		}
	}
}