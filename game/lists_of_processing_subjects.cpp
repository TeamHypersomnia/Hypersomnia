#include "lists_of_processing_subjects.h"
#include "game/components/force_joint_component.h"
#include "game/components/gui_element_component.h"
#include "game/components/input_receiver_component.h"
#include "game/components/trigger_query_detector_component.h"

std::vector<list_of_processing_subjects> lists_of_processing_subjects::find_matching(entity_id id) const {
	std::vector<list_of_processing_subjects> matching;

	if (id->has<components::animation>()) {
		matching.push_back(list_of_processing_subjects::WITH_ANIMATION);
	}
	if (id->has<components::behaviour_tree>()) {
		matching.push_back(list_of_processing_subjects::WITH_BEHAVIOUR_TREE);
	}
	if (id->has<components::camera>()) {
		matching.push_back(list_of_processing_subjects::WITH_CAMERA);
	}
	if (id->has<components::car>()) {
		matching.push_back(list_of_processing_subjects::WITH_CAR);
	}
	if (id->has<components::crosshair>()) {
		matching.push_back(list_of_processing_subjects::WITH_CROSSHAIR);
	}
	if (id->has<components::damage>()) {
		matching.push_back(list_of_processing_subjects::WITH_DAMAGE);
	}
	if (id->has<components::driver>()) {
		matching.push_back(list_of_processing_subjects::WITH_DRIVER);
	}
	if (id->has<components::force_joint>()) {
		matching.push_back(list_of_processing_subjects::WITH_FORCE_JOINT);
	}
	if (id->has<components::gui_element>()) {
		matching.push_back(list_of_processing_subjects::WITH_GUI_ELEMENT);
	}
	if (id->has<components::input_receiver>()) {
		matching.push_back(list_of_processing_subjects::WITH_INPUT_RECEIVER);
	}
	if (id->has<components::item_slot_transfers>()) {
		matching.push_back(list_of_processing_subjects::WITH_ITEM_SLOT_TRANSFERS);
	}
	if (id->has<components::melee>()) {
		matching.push_back(list_of_processing_subjects::WITH_MELEE);
	}
	if (id->has<components::movement>()) {
		matching.push_back(list_of_processing_subjects::WITH_MOVEMENT);
	}
	if (id->has<components::particle_group>()) {
		matching.push_back(list_of_processing_subjects::WITH_PARTICLE_GROUP);
	}
	if (id->has<components::pathfinding>()) {
		matching.push_back(list_of_processing_subjects::WITH_PATHFINDING);
	}
	if (id->has<components::position_copying>()) {
		matching.push_back(list_of_processing_subjects::WITH_POSITION_COPYING);
	}
	if (id->has<components::sentience>()) {
		matching.push_back(list_of_processing_subjects::WITH_SENTIENCE);
	}
	if (id->has<components::trace>()) {
		matching.push_back(list_of_processing_subjects::WITH_TRACE);
	}
	if (id->has<components::trigger_query_detector>()) {
		matching.push_back(list_of_processing_subjects::WITH_TRIGGER_QUERY_DETECTOR);
	}
	if (id->has<components::visibility>()) {
		matching.push_back(list_of_processing_subjects::WITH_VISIBILITY);
	}

	return matching;
}

void lists_of_processing_subjects::add_entity_to_matching_lists(entity_id id) {
	auto matching = find_matching(id);

	for (auto m : matching) {
		lists[m].push_back(id);
	}
}

void lists_of_processing_subjects::remove_entity_from_lists(entity_id id) {
	auto matching = find_matching(id);

	for (auto m : matching) {
		remove_element(lists[m], id);
	}
}

std::vector<entity_id> lists_of_processing_subjects::get(list_of_processing_subjects list) const {
	auto result = lists.at(list);

	erase_remove(result, [list](entity_id l) {
		return l->removed_from_processing_lists & (1 << unsigned long long(list));
	});

	return result;
}