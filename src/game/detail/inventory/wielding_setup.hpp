#include "game/detail/inventory/wielding_setup.h"

template <class A, class B>
entity_id get_wieldable_if_available(
	const A& gui_entity,
	const B& wieldable_entity
) {
	const auto character_capability = gui_entity.get_owning_transfer_capability();

	if (character_capability.dead()) {
		return {};
	}

	if (character_capability == wieldable_entity.get_owning_transfer_capability()) {
		if (!wieldable_entity.get_current_slot()->is_mounted_slot()) {
			return wieldable_entity.get_id();
		}
	}

	return {};
}

template <class E>
auto wielding_setup::make_viable_setup(const E& h) const {
	wielding_setup output;

	for (std::size_t i = 0; i < hand_selections.size(); ++i) {
		output.hand_selections[i] = get_wieldable_if_available(
			h, 
			h.get_cosmos()[hand_selections[i]]
		); 
	}

	return output;
}
