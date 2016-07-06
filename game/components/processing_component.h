#pragma once
#include <bitset>
#include "game/enums/processing_subjects.h"
#include "game/component_synchronizer.h"

namespace components {
	struct processing {
		typedef std::bitset<int(processing_subjects::LIST_COUNT)> bitset_type;
		static components::processing get_default(const_entity_handle);
		
		bool activated = true;

		bitset_type processing_subject_categories = 0;
	};
}

template<bool is_const>
class component_synchronizer<is_const, components::processing> : public component_synchronizer_base<is_const, components::processing> {
public:
	using component_synchronizer_base<is_const, components::processing>::component_synchronizer_base;

	bool is_in(processing_subjects) const;

	template<class = std::enable_if<!is_const>::type>
	void remove_from(processing_subjects) const;
	
	template<class = std::enable_if<!is_const>::type>
	void add_to(processing_subjects) const;
};