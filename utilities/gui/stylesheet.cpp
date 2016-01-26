#pragma once
#include "stylesheet.h"
#include "rect.h"

namespace augs {
	namespace graphics {
		namespace gui {
			stylesheet::style::style() 
				: color(rgba()), background_image(assets::texture_id::BLANK), border(solid_stroke(0)) {
			}

			stylesheet::style::style(const attribute<rgba>& c, 
				const attribute<assets::texture_id>& b,
				const attribute<solid_stroke>& br) 
				: color(c), background_image(b), border(br) {
					color.active = background_image.active = border.active = true;
			}

			stylesheet::style::operator material() const {
				return material(background_image.active ? background_image : assets::texture_id::BLANK, color.active ? color : rgba(255, 255, 255, 255));
			}

			stylesheet::stylesheet(const style& released, const style& hovered, const style& pushed, const style& focused) 
				: released(released), hovered(hovered), pushed(pushed), focused(focused), focus_flag(false), current_appearance(appearance::released) {}

			void stylesheet::update_appearance(rect::gui_event m) {
				auto app = map_event_to_appearance_type(m);
				
				if(m == rect::gui_event::focus)
					focus_flag = true;
				if(m == rect::gui_event::blur)
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

			stylesheet::appearance stylesheet::map_event_to_appearance_type(rect::gui_event m) {
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
}