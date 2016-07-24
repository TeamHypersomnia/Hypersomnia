#pragma once
#include <bitset>
#include "game/enums/processing_subjects.h"
#include "game/transcendental/component_synchronizer.h"

namespace components {
	struct processing {
		typedef std::bitset<int(processing_subjects::LIST_COUNT)> bitset_type;
		
		bool activated = true;

		bitset_type processing_subject_categories = 0;
		bitset_type disabled_categories = 0;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(activated),

				CEREAL_NVP(processing_subject_categories),
				CEREAL_NVP(disabled_categories)
			);
		}
		
		static components::processing get_default(const_entity_handle);
	};
}

template<bool is_const>
class basic_processing_synchronizer : public component_synchronizer_base<is_const, components::processing> {
public:
	using component_synchronizer_base<is_const, components::processing>::component_synchronizer_base;

	bool is_in(processing_subjects) const;
	components::processing::bitset_type get_disabled_categories() const;
	components::processing::bitset_type get_basic_categories() const;
};

template<>
class component_synchronizer<false, components::processing> : public basic_processing_synchronizer<false> {
	void resubstantialization() const;
public:
	using basic_processing_synchronizer<false>::basic_processing_synchronizer;

	void disable_in(processing_subjects) const;
	void enable_in(processing_subjects) const;
	void set_disabled_categories(components::processing::bitset_type) const;
	void set_basic_categories(components::processing::bitset_type) const;
};

template<>
class component_synchronizer<true, components::processing> : public basic_processing_synchronizer<true> {
public:
	using basic_processing_synchronizer<true>::basic_processing_synchronizer;
};