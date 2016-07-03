#include "processing_component.h"
#include "game/temporary_systems/processing_lists_system.h"

namespace components {
	//template<bool C>
	//processing_synchronizer<C>::processing_synchronizer(component_reference component, basic_entity_handle<C> handle) : component(component), handle(handle) {}
	//
	//template<bool C>
	//processing_synchronizer<C>::operator components::processing() const {
	//	return 
	//}

	template<bool C>
	bool component_synchronizer<C, components::processing>::is_in(processing_subjects list) const {
		return component.processing_subject_categories.test(int(list));
	}

	template<bool C>
	template <class = typename std::enable_if<!is_const>::type>
	processing_synchronizer<C>& processing_synchronizer<C>::operator=(const components::processing& p) {
		component.processing_subject_categories = p.processing_subject_categories;
		return *this;
	}

	template<bool C>
	template <class = typename std::enable_if<!is_const>::type>
	void processing_synchronizer<C>::skip_processing_in(processing_subjects list) {
		if (is_in(list)) {
			handle.get_cosmos().temporary_systems.get<processing_lists_system>().
		}

		component.processing_subject_categories.set(int(list), 0);
	}

	template<bool C>
	template <class = typename std::enable_if<!is_const>::type>
	void processing_synchronizer<C>::unskip_processing_in(processing_subjects) {
		component.processing_subject_categories.set(int(list), 1);
	}

	template class processing_synchronizer<false>;
	template class processing_synchronizer<true>;

}