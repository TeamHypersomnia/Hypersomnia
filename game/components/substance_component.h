#pragma once
#include "game/entity_handle_declaration.h"
#include "game/enums/processing_subjects.h"

namespace components {
	struct substance {
	private:
		unsigned long long processing_subject_categories = 0;

	public:
		bool is_in(processing_subjects) const;
		void set_processing_lists(unsigned long long);
		void skip_processing_in(processing_subjects);
		void unskip_processing_in(processing_subjects);
	};
}
