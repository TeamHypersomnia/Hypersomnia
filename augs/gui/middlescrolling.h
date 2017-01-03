#pragma once
#include "material.h"
#include "augs/math/rects.h"
#include "rect.h"
#include "augs/misc/delta.h"
#include "augs/window_framework/event.h"

namespace augs {
	namespace gui {
		template <class gui_element_polymorphic_id>
		struct middlescrolling {
			material mat;
			rects::wh<float> size = rects::wh<float>(25, 25);
			vec2i middlescroll_icon_position;
			gui_element_polymorphic_id subject;
			float speed_mult = 1.f;

			template<class C>
			void advance_elements(C context, const delta& dt) {
				if (context.alive(subject)) {
					context(subject, [&](auto& r) {
						r->set_scroll(r->get_scroll() + static_cast<vec2>(context.get_rect_world().last_state.mouse.pos - middlescroll_icon_position) * float(speed_mult*dt.in_milliseconds()));
					});
				}
			}

			template<class C>
			bool handle_new_raw_state(C context, const window::event::change& state) {
				if (context.alive(subject)) {
					if (state.msg == window::event::message::mdown || state.msg == window::event::message::mdoubleclick)
						subject.unset();

					return true;
				}

				return false;
			}

			template<class C>
			void draw(C context, draw_info in) const {
				if (context.alive(subject)) {
					rects::ltrb<float> scroller = rects::wh<float>(size);
					scroller.center(middlescroll_icon_position);
					draw_clipped_rect(mat, scroller, context, subject, in.v);
				}
			}
		};
	}
}