#include "appearance_detector.h"

namespace augs {
	namespace gui {
		void appearance_detector::update_appearance(rect::gui_event m) {
			auto app = map_event_to_appearance_type(m);

			if (m == rect::gui_event::focus)
				focus_flag = true;
			if (m == rect::gui_event::blur)
				focus_flag = false;

			if (app != appearance::unknown)
				current_appearance = app;
		}

		appearance_detector::appearance appearance_detector::map_event_to_appearance_type(rect::gui_event m) {
			if (m == rect::gui_event::hout
				|| m == rect::gui_event::lup
				|| m == rect::gui_event::loutup)
				return appearance::released;

			if (m == rect::gui_event::hover)
				return appearance::hovered;

			if (m == rect::gui_event::lpressed
				|| m == rect::gui_event::ldown
				|| m == rect::gui_event::ldoubleclick
				|| m == rect::gui_event::ltripleclick)
				return appearance::pushed;

			return appearance::unknown;
		}
	}
}