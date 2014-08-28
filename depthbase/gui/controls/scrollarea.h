#pragma once
#include "../rect.h"

namespace augs {
	namespace graphics {
		namespace gui {
			namespace controls {
				class scrollarea : public rect {
					void update_scroll_x();
					void update_scroll_y();
					using rect::children;
					using rect::parent;
				public:
					bool disappear_if_fits;

					virtual void event_proc(event_info) override;
					virtual void update_proc(group&) override;

					rect* origin;
					void align();

					bool enabled = true;
					bool is_needed();

					class slider : public rect {
						vec2<> val;
						friend class scrollarea;
					public:
						virtual void event_proc(event_info) override;
						
						int min_side;
						slider(int min_side);

					} *box;

					enum orientation {
						HORIZONTAL = 1,
						VERTICAL = 2,
						OMNI = (HORIZONTAL | VERTICAL)
					} flags;

					scrollarea(const math::rect_xywh& rc, rect* origin, slider* box, orientation flags);

					void draw_slider(draw_info);
				};
			}
		}
	}
}