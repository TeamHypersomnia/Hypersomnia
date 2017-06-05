#pragma once
#include "material.h"
#include "augs/math/rects.h"
#include "rect.h"
#include "augs/misc/delta.h"
#include "augs/window_framework/event.h"

namespace augs {
	namespace gui {
		template <class gui_element_variant_id>
		struct middlescrolling {
			material mat;
			vec2 size = vec2(25, 25);
			vec2i middlescroll_icon_position;
			gui_element_variant_id subject;
			float speed_mult = 1.f;

			template<class C>
			void advance_elements(const C context, const delta& dt) {
				if (context.alive(subject)) {
					context(subject, [&](auto& r) {
						r->set_scroll(r->get_scroll() + static_cast<vec2>(context.get_rect_world().last_state.mouse.pos - middlescroll_icon_position) * float(speed_mult*dt.in_milliseconds()));
					});
				}
			}

			template<class C>
			bool handle_new_raw_state(const C context, const window::event::change& state) {
				if (context.alive(subject)) {
					if (state.msg == window::event::message::mdown || state.msg == window::event::message::mdoubleclick)
						subject = gui_element_variant_id();

					return true;
				}

				return false;
			}

			template<class C>
			void draw(const C context, draw_info in) const {
				if (context.alive(subject)) {
					auto scroller = ltrb(vec2(), size);
					scroller.center(middlescroll_icon_position);
					draw_clipped_rect(mat, scroller, context, subject, in.v);
				}
			}
		};
	}
}