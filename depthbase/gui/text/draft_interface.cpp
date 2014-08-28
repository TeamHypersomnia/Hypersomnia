#pragma once
#include "draft_interface.h"
namespace augs {
	namespace graphics {
		namespace gui {
			namespace text {
					
			   abstract_draft::abstract_draft() : update_str(true) {

			   }

				void abstract_draft::need_redraw() {
					update_str = true;
				}

				void abstract_draft::guarded_redraw() {
					if(update_str) {
						draft().draw(get_str());
						update_str = false;
					}
				}
				
				draft_redrawer::draft_redrawer(const fstr& _str, const drafter& _draft) : _str(_str), _draft(_draft) {
				}

				drafter& draft_redrawer::draft() {
					need_redraw();
					return _draft;
				}

				const drafter& draft_redrawer::get_draft() {
					guarded_redraw();
					return _draft;
				}

				fstr& draft_redrawer::str() {
					need_redraw();
					return _str;
				}

				const fstr& draft_redrawer::get_str() const {
					return _str;
				}
			}
		}
	}
}