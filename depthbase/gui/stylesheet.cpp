#pragma once
#include "stylesheet.h"
#include "rect.h"

namespace augs {
	namespace graphics {
		namespace gui {
			stylesheet::style::style() 
				: color(rgba()), background_image(gui::null_texture), border(solid_stroke(0)) {
			}

			stylesheet::style::style(const attribute<rgba>& c, 
				const attribute<augs::texture*>& b, 
				const attribute<solid_stroke>& br) 
				: color(c), background_image(b), border(br) {
					color.active = background_image.active = border.active = true;
			}

			stylesheet::style::operator material() const {
				return material(background_image.active ? background_image : null_texture, color.active ? color : rgba(255, 255, 255, 255));
			}

			stylesheet::stylesheet(const style& released, const style& hovered, const style& pushed, const style& focused) 
				: released(released), hovered(hovered), pushed(pushed), focused(focused), focus_flag(false), current_appearance(appearance::released) {}

			void stylesheet::update_appearance(rect::event m) {
				auto app = map_event_to_appearance_type(m);
				
				if(m == rect::event::focus)
					focus_flag = true;
				if(m == rect::event::blur)
					focus_flag = false;

				if(app != appearance::unknown) 
					current_appearance = app;
			}

			stylesheet::style stylesheet::get_style() const {
				style base(focus_flag ? (style(released) = focused) : released);

				/* operator='s take care of copying only with active members */
				switch(current_appearance) {
				case appearance::released: return base; break;
				case appearance::hovered: return style(base) = hovered;  break;
				case appearance::pushed: return (style(base) = hovered) = pushed;   break;
				default: return base;
				}

			}

			stylesheet::appearance stylesheet::map_event_to_appearance_type(rect::event m) {
				if (m == rect::event::hout
					|| m == rect::event::lup
					|| m == rect::event::loutup)
					return appearance::released;

				if (m == rect::event::hover)
					return appearance::hovered;

				if (m == rect::event::lpressed
					|| m == rect::event::ldown
					|| m == rect::event::ldoubleclick
					|| m == rect::event::ltripleclick)
					return appearance::pushed;

				return appearance::unknown;
			}
		}
	}
}