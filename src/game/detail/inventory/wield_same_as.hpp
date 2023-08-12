#pragma once


template <class E, class C>
auto wield_same_as(const E& similar_wielded, const logic_step step, const C& character) {
	auto wielding = wielding_setup::bare_hands();

	bool found = false;

	character.for_each_contained_item_recursive(
		[&](const auto candidate) {
			if (entity_flavour_id(similar_wielded.get_flavour_id()) == entity_flavour_id(candidate.get_flavour_id())) {
				found = true;

				wielding.hand_selections[0] = candidate;

				return recursive_callback_result::ABORT;
			}

			return recursive_callback_result::CONTINUE_AND_RECURSE;
		}
	);

	if (found) {
		::perform_wielding(
			step,
			character,
			wielding
		);
	}

}

template <class E, class C>
auto wield_same_melee_as(const E& similar_wielded, const logic_step step, const C& character) {
	auto wielding = wielding_setup::bare_hands();

	bool found = false;

	entity_id fallback;

	character.for_each_contained_item_recursive(
		[&](const auto candidate) {
			if (entity_flavour_id(similar_wielded.get_flavour_id()) == entity_flavour_id(candidate.get_flavour_id())) {
				found = true;

				wielding.hand_selections[0] = candidate;

				return recursive_callback_result::ABORT;
			}
			else {
				if (candidate.template has<components::melee>()) {
					fallback = candidate.get_id();
				}
			}

			return recursive_callback_result::CONTINUE_AND_RECURSE;
		}
	);

	if (found) {
		::perform_wielding(
			step,
			character,
			wielding,
			false
		);
	}
	else if (fallback.is_set()) {
		wielding.hand_selections[0] = fallback;

		::perform_wielding(
			step,
			character,
			wielding,
			false
		);
	}
}
