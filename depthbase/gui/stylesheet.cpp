#pragma once
#include "stylesheet.h"
#include "rect.h"

namespace augs {
	namespace graphics {
		namespace gui {
			stylesheet::style::style() 
				: color(pixel_32()), background_image(gui::null_texture), border(solid_stroke(0)) {
			}

			stylesheet::style::style(const attribute<pixel_32>& c, 
				const attribute<augs::texture*>& b, 
				const attribute<solid_stroke>& br) 
				: color(c), background_image(b), border(br) {
					color.active = background_image.active = border.active = true;
			}

			stylesheet::style::operator material() const {
				return material(background_image.active ? background_image : null_texture, color.active ? color : pixel_32(255, 255, 255, 255));
			}

			stylesheet::stylesheet(const style& released, const style& hovered, const style& pushed, const style& focused) 
				: released(released), hovered(hovered), pushed(pushed), focused(focused), focus_flag(false), current_appearance(rect::appearance::released) {}

			void stylesheet::update_appearance(rect::event m) {
				auto app = rect::get_appearance(m);
				
				if(m == rect::event::focus)
					focus_flag = true;
				if(m == rect::event::blur)
					focus_flag = false;

				if(app != rect::appearance::unknown) 
					current_appearance = app;
			}

			stylesheet::style stylesheet::get_style() const {
				style base(focus_flag ? (style(released) = focused) : released);

				/* operator='s take care of copying only with active members */
				switch(current_appearance) {
				case rect::appearance::released: return base; break;
				case rect::appearance::hovered: return style(base) = hovered;  break;
				case rect::appearance::pushed: return (style(base) = hovered) = pushed;   break;
				default: return base;
				}

			}
		}
	}
}