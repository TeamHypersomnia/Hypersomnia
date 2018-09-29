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

template <class I>
template <class E>
auto basic_wielding_setup<I>::make_viable_setup(const E& h) const {
	basic_wielding_setup<I> output;

	for (std::size_t i = 0; i < hand_selections.size(); ++i) {
		output.hand_selections[i] = get_wieldable_if_available(
			h, 
			h.get_cosmos()[hand_selections[i]]
		); 
	}

	return output;
}

template <class I>
template <class C>
bool basic_wielding_setup<I>::is_akimbo(const C& cosm) const {
	for (const auto& s : hand_selections) {
		if (cosm[s].dead()) {
			return false;
		}
	}

	return true;
}

template <class I>
template <class E>
basic_wielding_setup<I> basic_wielding_setup<I>::from_current(const E& character_entity) {
	basic_wielding_setup<I> output;

	for (std::size_t i = 0; i < output.hand_selections.size(); ++i) {
		output.hand_selections[i] = character_entity.get_if_any_item_in_hand_no(i);
	}

	return output;
}
