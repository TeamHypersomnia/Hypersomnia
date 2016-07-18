#include "processing_component.h"
#include "game/temporary_systems/processing_lists_system.h"
#include "game/entity_handle.h"
#include "game/cosmos.h"
#include "game/types_specification/all_component_includes.h"

typedef components::processing P;

P P::get_default(const_entity_handle id) {
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
	if (id.has<components::rotation_copying>()) {
		matching.push_back(processing_subjects::WITH_ROTATION_COPYING);
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

	P result;

	for (auto m : matching)
		result.processing_subject_categories.set(int(m));

	return result;
}

template<bool C>
bool basic_processing_synchronizer<C>::is_in(processing_subjects list) const {
	return component.processing_subject_categories.test(int(list)) && !component.disabled_categories.test(int(list));
}

void component_synchronizer<false, P>::disable_in(processing_subjects list) const {
	component.disabled_categories.set(int(list), 1);
	complete_resubstantialization();
}

void component_synchronizer<false, P>::enable_in(processing_subjects list) const {
	component.disabled_categories.set(int(list), 0);
	complete_resubstantialization();
}

template<bool C>
P::bitset_type basic_processing_synchronizer<C>::get_disabled_categories() const {
	return component.disabled_categories;
}

template<bool C>
P::bitset_type basic_processing_synchronizer<C>::get_basic_categories() const {
	return component.processing_subject_categories;
}

void component_synchronizer<false, P>::set_disabled_categories(P::bitset_type categories) const {
	component.disabled_categories = categories;
	complete_resubstantialization();
}

void component_synchronizer<false, P>::set_basic_categories(P::bitset_type categories) const {
	component.processing_subject_categories = categories;
	complete_resubstantialization();
}

template class basic_processing_synchronizer<false>;
template class basic_processing_synchronizer<true>;