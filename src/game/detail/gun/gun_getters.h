#pragma once

template <class E>
bool requires_two_hands_to_chamber(const E& gun_entity) {
	if (const auto gun_def = gun_entity.template find<invariants::gun>()) {
		return !gun_def->allow_chambering_with_akimbo;
	}

	return false;
}

template <class E>
bool chambering_in_order(const E& gun_entity) {
	if (const auto chamber_slot = gun_entity[slot_function::GUN_CHAMBER]) {
		if (chamber_slot.is_empty_slot()) {
			if (const auto mag_chamber = gun_entity[slot_function::GUN_CHAMBER_MAGAZINE]) {
				if (mag_chamber.has_items()) {
					return true;
				}
			}

			if (const auto mag_slot = gun_entity[slot_function::GUN_DETACHABLE_MAGAZINE]) {
				if (const auto mag_inside = mag_slot.get_item_if_any()) {
					if (0 != count_charges_in_deposit(mag_inside)) {
						return true;
					}
				}
			}
		}
	}

	return false;
}

