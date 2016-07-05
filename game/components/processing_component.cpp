#include "processing_component.h"
#include "game/temporary_systems/processing_lists_system.h"
#include "game/entity_handle.h"

components::processing components::processing::get_default(const_entity_handle id) {
	std::vector<processing_subjects> matching;

	if (id.has<components::animation>()) {
		matching.push_back(processing_subjects::WITH_ANIMATION);
	}
	if (id.has<components::behaviour_tree>()) {
		matching.push_back(processing_subjects::WITH_BEHAVIOUR_TREE);
	}
	if (id.has<components::camera>()) {
		matching.push_back(processing_subjects::WITH_CAMERA);
	}
	if (id.has<components::car>()) {
		matching.push_back(processing_subjects::WITH_CAR);
	}
	if (id.has<components::crosshair>()) {
		matching.push_back(processing_subjects::WITH_CROSSHAIR);
	}
	if (id.has<components::damage>()) {
		matching.push_back(processing_subjects::WITH_DAMAGE);
	}
	if (id.has<components::driver>()) {
		matching.push_back(processing_subjects::WITH_DRIVER);
	}
	if (id.has<components::force_joint>()) {
		matching.push_back(processing_subjects::WITH_FORCE_JOINT);
	}
	if (id.has<components::gui_element>()) {
		matching.push_back(processing_subjects::WITH_GUI_ELEMENT);
	}
	if (id.has<components::gun>()) {
		matching.push_back(processing_subjects::WITH_GUN);
	}
	if (id.has<components::input_receiver>()) {
		matching.push_back(processing_subjects::WITH_INPUT_RECEIVER);
	}
	if (id.has<components::item_slot_transfers>()) {
		matching.push_back(processing_subjects::WITH_ITEM_SLOT_TRANSFERS);
	}
	if (id.has<components::melee>()) {
		matching.push_back(processing_subjects::WITH_MELEE);
	}
	if (id.has<components::movement>()) {
		matching.push_back(processing_subjects::WITH_MOVEMENT);
	}
	if (id.has<components::particle_group>()) {
		matching.push_back(processing_subjects::WITH_PARTICLE_GROUP);
	}
	if (id.has<components::pathfinding>()) {
		matching.push_back(processing_subjects::WITH_PATHFINDING);
	}
	if (id.has<components::position_copying>()) {
		matching.push_back(processing_subjects::WITH_POSITION_COPYING);
	}
	if (id.has<components::sentience>()) {
		matching.push_back(processing_subjects::WITH_SENTIENCE);
	}
	if (id.has<components::trace>()) {
		matching.push_back(processing_subjects::WITH_TRACE);
	}
	if (id.has<components::trigger_query_detector>()) {
		matching.push_back(processing_subjects::WITH_TRIGGER_QUERY_DETECTOR);
	}
	if (id.has<components::visibility>()) {
		matching.push_back(processing_subjects::WITH_VISIBILITY);
	}

	components::processing result;

	for (auto m : matching)
		result.processing_subject_categories.set(int(m));

	return result;
}

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

template class component_synchronizer<false, components::processing>;
template class component_synchronizer<true, components::processing>;
