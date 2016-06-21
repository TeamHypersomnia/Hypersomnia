#include "lists_of_processing_subjects.h"
#include "game/components/force_joint_component.h"
#include "game/components/gui_element_component.h"
#include "game/components/input_receiver_component.h"
#include "game/components/trigger_query_detector_component.h"
#include "game/cosmos.h"
#include "game/entity_handle.h"

std::vector<processing_subjects> lists_of_processing_subjects::find_matching(const_entity_handle id) const {
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

	return matching;
}

void lists_of_processing_subjects::add_entity_to_matching_lists(const_entity_handle id) {
	auto matching = find_matching(id);

	for (auto m : matching) {
		lists[m].push_back(id);
	}
}

void lists_of_processing_subjects::remove_entity_from_lists(const_entity_handle id) {
	auto matching = find_matching(id);

	for (auto m : matching) {
		remove_element(lists[m], id.get_id());
	}
}

std::vector<entity_handle> lists_of_processing_subjects::get(processing_subjects list, cosmos& cosmos) const {
	const auto& subjects = lists.at(list);
	std::vector<entity_handle> handles;

	//
	//erase_remove(result, [list](entity_id l) {
	//	return l->removed_from_processing_subjects & (1 << unsigned long long(list));
	//});

	for (auto s : subjects) {
		auto handle = cosmos.get_handle(s);

		if (handle.removed_from_processing_subjects & (1 << unsigned long long(list))) {
			handles.emplace_back(handle);
		}
	}

	return handles;
}

bool lists_of_processing_subjects::is_in(entity_id id, processing_subjects) const {

}

std::vector<entity_handle> get(processing_subjects, cosmos&) const;
std::vector<const_entity_handle> get(processing_subjects, const cosmos&) const;