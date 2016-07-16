#include "middlescrolling.h"
#include "misc/pool.h"
#include "rect_world.h"

namespace augs {
	namespace gui {
		void middlescrolling::perform_logic_step(rect_pool& rects, fixed_delta dt, window::event::state state) {
			auto r = rects[subject];

			if (r.alive())
				r.get().scroll += static_cast<vec2>(state.mouse.pos - pos) * float(speed_mult*dt.in_milliseconds());
		}

		bool middlescrolling::handle_new_raw_state(rect_pool& rects, window::event::state state) {
			if (rects[subject].alive()) {
				if (state.msg == window::event::mdown || state.msg == window::event::mdoubleclick)
					subject = rect_id();
				
				return true;
			}

			return false;
		}

		void middlescrolling::draw_triangles(const rect_pool& rects, draw_info in) const {
			auto r = rects[subject];

			if (r.alive()) {
				rects::ltrb<float> scroller = rects::wh<float>(size);
				scroller.center(pos);
				draw_clipped_rectangle(mat, scroller, r.get(), in.v);
			}
		}
	}
}
