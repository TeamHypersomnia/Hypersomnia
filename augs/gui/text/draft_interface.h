#pragma once
#include "drafter.h"
namespace augs {
	namespace gui {
		namespace text {
			class abstract_draft {
				bool update_str;

			public:
				abstract_draft();

				void need_redraw();
				void guarded_redraw();

				virtual 	  formatted_string& str() = 0;
				virtual const formatted_string& get_str() const = 0;
				virtual 	  drafter& draft() = 0;
				virtual const drafter& get_draft() = 0;
			};

			class draft_redrawer : public abstract_draft {
				formatted_string _str;
				drafter _draft;
			public:

				draft_redrawer(const formatted_string& = formatted_string(), const drafter& = drafter());

				formatted_string& str() override;
				const formatted_string& get_str() const override;
				drafter& draft() override;
				const drafter& get_draft() override;
			};
		}
	}
}