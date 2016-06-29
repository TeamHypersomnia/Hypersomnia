#pragma once
#include <bitset>
#include "game/enums/processing_subjects.h"
#include "game/component_synchronizer.h"

namespace components {
	struct processing {
		typedef std::bitset<int(processing_subjects::LIST_COUNT)> bitset_type;

		bitset_type processing_subject_categories = 0;
	};

	template<bool is_const>
	class processing_synchronizer : public component_synchronizer<processing, is_const> {
	public:
		bool is_in(processing_subjects) const;

		template <class = typename std::enable_if<!is_const>::type>
		processing_synchronizer& operator=(const components::processing&);

		template <class = typename std::enable_if<!is_const>::type>
		void skip_processing_in(processing_subjects);

		template <class = typename std::enable_if<!is_const>::type>
		void unskip_processing_in(processing_subjects);
	};
}