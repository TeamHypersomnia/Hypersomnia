#include "draft_redrawer.h"
namespace augs {
	namespace gui {
		namespace text {
			void draft_redrawer::set_needs_redraw() {
				needs_redraw = true;
			}

			void draft_redrawer::guarded_redraw() {
				if (needs_redraw) {
					draft().draw(get_str());
					needs_redraw = false;
				}
			}

			drafter& draft_redrawer::draft() {
				set_needs_redraw();
				return _draft;
			}

			const drafter& draft_redrawer::get_draft() {
				guarded_redraw();
				return _draft;
			}

			formatted_string& draft_redrawer::str() {
				set_needs_redraw();
				return _str;
			}

			const formatted_string& draft_redrawer::get_str() const {
				return _str;
			}
		}
	}
}