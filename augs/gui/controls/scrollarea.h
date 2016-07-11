#pragma once
#include "gui/rect.h"

namespace augs {
	namespace gui {
		namespace controls {
			class scrollarea : public rect {
				void update_scroll_x();
				void update_scroll_y();
				using rect::children;
				using rect::parent;
			public:
				bool disappear_if_fits;

				virtual void consume_gui_event(event_info) override;
				virtual void perform_logic_step(rect_world&) override;

				rect_id origin;
				void align();

				bool enabled = true;
				bool is_needed();

				class slider : public rect {
					vec2 val;
					friend class scrollarea;
				public:
					virtual void consume_gui_event(event_info) override;

					int min_side;
					slider(int min_side);

				} *box;

				enum orientation {
					HORIZONTAL = 1,
					VERTICAL = 2,
					OMNI = (HORIZONTAL | VERTICAL)
				} flags;

				scrollarea(const rects::xywh<float>& rc, rect_id origin, slider* box, orientation flags);
			};
		}
	}
}