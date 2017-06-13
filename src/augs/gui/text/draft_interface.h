#pragma once
#include "drafter.h"
namespace augs {
	namespace gui {
		namespace text {
			class draft_redrawer {
				formatted_string _str;
				drafter _draft;
				bool needs_redraw = true;
			public:
				void set_needs_redraw();
				void guarded_redraw();

				formatted_string& str();
				const formatted_string& get_str() const;
				drafter& draft();
				const drafter& get_draft();
			};
		}
	}
}