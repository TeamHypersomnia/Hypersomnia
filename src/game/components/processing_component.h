#pragma once
#include "game/enums/processing_subjects.h"
#include "game/transcendental/component_synchronizer.h"

#include "augs/pad_bytes.h"
#include "augs/misc/enum/enum_boolset.h"

namespace components {
	struct processing {
		static constexpr bool is_synchronized = true;
		static constexpr bool is_always_present = true;

		using flagset_type = augs::enum_boolset<processing_subjects>;
		
		// GEN INTROSPECTOR struct components::processing
		bool activated = true;
		pad_bytes<3> pad;

		flagset_type processing_subject_categories;
		flagset_type disabled_categories;
		// END GEN INTROSPECTOR

		static components::processing get_default(const const_entity_handle);
	};
}

template<bool is_const>
class basic_processing_synchronizer : public component_synchronizer_base<is_const, components::processing> {
protected:
	using base = component_synchronizer_base<is_const, components::processing>;
	using base::handle;
public:
	using base::component_synchronizer_base;
	using base::get_raw_component;

	bool is_activated() const;
	bool is_in(const processing_subjects) const;
	components::processing::flagset_type get_disabled_categories() const;
	components::processing::flagset_type get_basic_categories() const;
};

template<>
class component_synchronizer<false, components::processing> : public basic_processing_synchronizer<false> {
	void reinfer_caches() const;
public:
	using basic_processing_synchronizer<false>::basic_processing_synchronizer;

	void disable_in(const processing_subjects) const;
	void enable_in(const processing_subjects) const;
	void set_disabled_categories(const components::processing::flagset_type&) const;
	void set_basic_categories(const components::processing::flagset_type&) const;
};

template<>
class component_synchronizer<true, components::processing> : public basic_processing_synchronizer<true> {
public:
	using basic_processing_synchronizer<true>::basic_processing_synchronizer;
};