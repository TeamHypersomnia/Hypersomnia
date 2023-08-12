#include "game/detail/inventory/wielding_setup.h"
#include "game/detail/weapon_like.h"
#include "game/detail/gun/ammo_logic.h"

template <class I>
template <class C>
bool basic_wielding_setup<I>::equivalent_to(const C& cosm, const basic_wielding_setup<I>& b) const {
	auto both_same_stackable_flavour = [](auto aa, auto bb) {
		return aa.alive() && bb.alive() && ::is_stackable_in_hotbar(aa) && aa.get_flavour_id() == bb.get_flavour_id();
	};

	const auto equal_0 = hand_selections[0] == b.hand_selections[0] || both_same_stackable_flavour(cosm[hand_selections[0]], cosm[b.hand_selections[0]]);
	const auto equal_1 = hand_selections[1] == b.hand_selections[1] || both_same_stackable_flavour(cosm[hand_selections[1]], cosm[b.hand_selections[1]]);

	return equal_0 && equal_1;
}

template <class I>
template <class C>
bool basic_wielding_setup<I>::is_bare_hands(const C& cosm) const {
	for (std::size_t i = 0; i < hand_selections.size(); ++i) {
		if (cosm[hand_selections[i]]) {
			return false;
		}
	}

	return true;
}

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
auto basic_wielding_setup<I>::make_viable_setup(const E& gui_entity) const {
	auto output = basic_wielding_setup<I>::bare_hands();

	for (std::size_t i = 0; i < hand_selections.size(); ++i) {
		if (const auto candidate_wieldable = gui_entity.get_cosmos()[hand_selections[i]]) {
			if (::is_weapon_like(candidate_wieldable)) {
				output.hand_selections[i] = get_wieldable_if_available(gui_entity, candidate_wieldable);
			}
		}

	}

	return output;
}

template <class I>
template <class E>
I basic_wielding_setup<I>::get_other_than(const E& wielded_item) const {
	for (std::size_t i = 0; i < hand_selections.size(); ++i) {
		const auto& other = hand_selections[i];

		if (other != wielded_item) {
			return other;
		}
	}

	return {};
}

template <class I>
template <class C>
std::size_t basic_wielding_setup<I>::least_weapon_index(const C& cosm) const {
	for (std::size_t i = 0; i < hand_selections.size(); ++i) {
		const auto selection = cosm[hand_selections[i]];

		if (selection.dead()) {
			return i;
		}
	}

	for (std::size_t i = 0; i < hand_selections.size(); ++i) {
		const auto selection = cosm[hand_selections[i]];

		if (nullptr == selection.template find<components::gun>()) {
			return i;
		}
	}

	return 0;
}

template <class I>
template <class E>
void basic_wielding_setup<I>::clear_hand_with(const E& wielded_item) {
	for (std::size_t i = 0; i < hand_selections.size(); ++i) {
		if (hand_selections[i] == wielded_item) {
			hand_selections[i] = {};
		}
	}
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
	auto output = basic_wielding_setup<I>::bare_hands();

	for (std::size_t i = 0; i < output.hand_selections.size(); ++i) {
		const auto candidate_wieldable = character_entity.get_if_any_item_in_hand_no(i);

		if (candidate_wieldable) {
			if (::is_weapon_like(candidate_wieldable)) {
				output.hand_selections[i] = candidate_wieldable;
			}
		}
	}

	return output;
}

template <class I>
template <class C, class F>
decltype(auto) basic_wielding_setup<I>::on_more_recent_item(C& cosm, F&& callback) const {
	const auto entity_0 = cosm[hand_selections[0]];
	const auto entity_1 = cosm[hand_selections[1]];

	if (entity_0 && entity_1) {
		auto when = [&](const auto& it) {
			return it.when_last_transferred();
		};

		if (when(entity_0) > when(entity_1)) {
			return callback(entity_0, 0);
		}

		if (when(entity_1) > when(entity_0)) {
			return callback(entity_1, 1);
		}
	}

	return callback(std::nullopt, std::nullopt);
}
