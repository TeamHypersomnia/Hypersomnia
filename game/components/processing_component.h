#pragma once
#include <bitset>
#include "game/enums/processing_subjects.h"
#include "game/transcendental/component_synchronizer.h"

#include "augs/padding_byte.h"
#include "augs/misc/enum_bitset.h"

namespace components {
	struct processing : synchronizable_component {
		typedef augs::enum_bitset<processing_subjects> bitset_type;
		
		bool activated = true;
		padding_byte pad[3];

		bitset_type processing_subject_categories;
		bitset_type disabled_categories;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(activated),

				CEREAL_NVP(processing_subject_categories),
				CEREAL_NVP(disabled_categories)
			);
		}
		
		static components::processing get_default(const const_entity_handle&);
	};
}

template<bool is_const>
class basic_processing_synchronizer : public component_synchronizer_base<is_const, components::processing> {
public:
	using component_synchronizer_base<is_const, components::processing>::component_synchronizer_base;

	bool is_activated() const;
	bool is_in(const processing_subjects) const;
	components::processing::bitset_type get_disabled_categories() const;
	components::processing::bitset_type get_basic_categories() const;
};

template<>
class component_synchronizer<false, components::processing> : public basic_processing_synchronizer<false> {
	void resubstantiation() const;
public:
	using basic_processing_synchronizer<false>::basic_processing_synchronizer;

	void disable_in(const processing_subjects) const;
	void enable_in(const processing_subjects) const;
	void set_disabled_categories(const components::processing::bitset_type&) const;
	void set_basic_categories(const components::processing::bitset_type&) const;
};

template<>
class component_synchronizer<true, components::processing> : public basic_processing_synchronizer<true> {
public:
	using basic_processing_synchronizer<true>::basic_processing_synchronizer;
};