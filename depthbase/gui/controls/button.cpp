#pragma once
#include "button.h"
#include "../../../event.h"
#include "../system.h"

namespace db {
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

				void button::event_proc(event_info m) {
					switch(m) {
					case event::lclick: if(on_click) on_click(); break;
					case event::hover: if(on_hover) on_hover(); break;
					case event::ldown: if(on_lmousedown) on_mousedown(); break;
					case event::lup: if(on_lmouseup) on_mouseup(); break;
					case event::keydown: if(m.owner.owner.events.key == db::event::keys::ENTER) on_click(); break;
					default: break;
					}
					handle_focus(m);
					handle_tab(m);
					handle_arrows(m);
				}

				
				text_button::text_button(const button& b, const text::fstr& f) : button(b), label(rect(), f) {
					center();
				}

				text_button::text_button(const button& b, math::point p, const text::fstr& f) : button(b), label(rect_xywh(p.x, p.y, 0, 0), f) {
				}

				void text_button::get_member_children(vector<rect*>& v) {
					v.push_back(&label);
				}
				
				void text_button::center() {
					label.center(rc);
				}

			};
		}
	}
}
