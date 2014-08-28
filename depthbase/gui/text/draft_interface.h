#pragma once
#include "drafter.h"
namespace db {
	namespace graphics {
		namespace gui {
			namespace text {
				class abstract_draft {
					bool update_str;
				
				public:
					abstract_draft();

					void need_redraw();
					void guarded_redraw();

					virtual 	  fstr& str() = 0;
					virtual const fstr& get_str() const = 0;
					virtual 	  drafter& draft() = 0;
					virtual const drafter& get_draft() = 0;
				};

				class draft_redrawer : public abstract_draft {
					fstr _str;
					drafter _draft;
				public:

					draft_redrawer(const fstr& = fstr(), const drafter& = drafter());

						  fstr& str() override;
					const fstr& get_str() const override;
						  drafter& draft() override;
					const drafter& get_draft() override;
				};
			}
		}
	}
}