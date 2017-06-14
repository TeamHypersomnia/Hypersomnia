#include "appearance_detector.h"

namespace augs {
	namespace gui {
		void appearance_detector::update_appearance(const gui_event m) {
			appearance app = appearance::unknown;

			if (!remain_pushed_if_mouse_leaves) {
				if (
					m == gui_event::hoverlost
					|| m == gui_event::lup
					|| m == gui_event::loutup
					|| m == gui_event::rup
					|| m == gui_event::routup
				) {
					app = appearance::released;
				}
			}
			else {
				if (
					m == gui_event::lup
					|| m == gui_event::loutup
					|| m == gui_event::rup
					|| m == gui_event::routup
				) {
					app = appearance::released;
				}
			}

			if (m == gui_event::hover) {
				is_hovered = true;
			}

			if (
				m == gui_event::lpressed
				|| m == gui_event::ldown
				|| m == gui_event::ldoubleclick
				|| m == gui_event::ltripleclick
				|| m == gui_event::rpressed
				|| m == gui_event::rdown
				|| m == gui_event::rdoubleclick
			) {
				app = appearance::pushed;
			}

			if (m == gui_event::focus) {
				focus_flag = true;
			}
			if (m == gui_event::blur) {
				focus_flag = false;
			}
			if (m == gui_event::hoverlost) {
				is_hovered = false;
			}

			if (app != appearance::unknown) {
				current_appearance = app;
			}
		}
	}
}