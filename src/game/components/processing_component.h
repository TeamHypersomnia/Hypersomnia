#pragma once
#include "game/enums/processing_subjects.h"
#include "game/transcendental/component_synchronizer.h"

#include "augs/pad_bytes.h"
#include "augs/misc/enum/enum_boolset.h"
#include "game/transcendental/entity_handle_declaration.h"

namespace components {
	struct processing {
		static constexpr bool is_synchronized = true;
		static constexpr bool is_always_present = true;

		using flagset_type = augs::enum_boolset<processing_subjects>;
		
		// GEN INTROSPECTOR struct components::processing
		flagset_type processing_subject_categories;
		// END GEN INTROSPECTOR

		bool operator==(const processing& b) const {
			return 
				processing_subject_categories == b.processing_subject_categories
			;
		}

		static components::processing get_default(const const_entity_handle);
	};
}

template <class E>
class component_synchronizer<E, components::processing> : public synchronizer_base<E, components::processing> {
protected:
	using base = synchronizer_base<E, components::processing>;
	using base::handle;

	void infer_caches() const{
		handle.get_cosmos().get_solvable_inferred({}).processing_lists.infer_cache_for(handle);
	}

	using base::get_writable;
public:
	using base::synchronizer_base;
	using base::get_raw_component;

	bool is_in(const processing_subjects list) const{
		return get_raw_component().processing_subject_categories.test(list);
	}

	auto get_basic_categories() const{
		return get_raw_component().processing_subject_categories;
	}

	void set_basic_categories(const components::processing::flagset_type& categories) const{
		get_writable().processing_subject_categories = categories;
		infer_caches();
	}
};