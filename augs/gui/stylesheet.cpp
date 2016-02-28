#pragma once
#include "stylesheet.h"
#include "rect.h"

namespace augs {
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
			: released(released), hovered(hovered), pushed(pushed), focused(focused) {}

		stylesheet::style stylesheet::get_style() const {
			style base(focus_flag ? (style(released) = focused) : released);

			/* operator='s take care of copying only with active members */
			switch (current_appearance) {
			case appearance::released: return base; break;
			case appearance::hovered: return style(base) = hovered;  break;
			case appearance::pushed: return (style(base) = hovered) = pushed;   break;
			default: return base;
			}
		}
	}
}