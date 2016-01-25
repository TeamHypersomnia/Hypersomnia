#pragma once
#include "button.h"
#include "window_framework/event.h"
#include "../gui_world.h"

namespace augs {
	namespace graphics {
		namespace gui {
			namespace controls {
				std::function<void()> on_click;
				std::function<void()> on_hover;
				std::function<void()> on_mousedown;
				std::function<void()> on_mouseup;

				button::button(const rect& r, const std::function<void()>& on_click,
					const std::function<void()>& on_hover,
					const std::function<void()>& on_lmousedown,
					const std::function<void()>& on_lmouseup) : 
				rect(r), 
					on_click(on_click),
					on_hover(on_hover),
					on_lmousedown(on_lmousedown),
					on_lmouseup(on_lmouseup) {}

				void button::consume_gui_event(event_info m) {
					switch(m) {
					case gui_event::lclick: if(on_click) on_click(); break;
					case gui_event::hover: if(on_hover) on_hover(); break;
					case gui_event::ldown: if(on_lmousedown) on_mousedown(); break;
					case gui_event::lup: if(on_lmouseup) on_mouseup(); break;
					case gui_event::keydown: if(m.owner.state.key == augs::window::event::keys::ENTER) on_click(); break;
					default: break;
					}
					try_to_make_this_rect_focused(m);
					focus_next_rect_by_tab(m);
					focus_next_rect_by_arrows(m);
				}

				
				text_button::text_button(const button& b, const text::fstr& f) : button(b), label(rect(), f) {
					center();
				}

				text_button::text_button(const button& b, vec2i p, const text::fstr& f) : button(b), label(rects::xywh<float>(p.x, p.y, 0, 0), f) {
				}

				void text_button::get_member_children(std::vector<rect*>& v) {
					v.push_back(&label);
				}
				
				void text_button::center() {
					label.center(rc);
				}

			};
		}
	}
}
