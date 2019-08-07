#pragma once

int count_charges_inside(const const_inventory_slot_handle id);
int count_charges_in_deposit(const const_entity_handle item);

struct ammunition_information {
	unsigned total_charges = 0;
	real32 total_ammo_space = 0.f;
	real32 available_ammo_space = 0.f;

	real32 get_ammo_ratio() const {
		return 1 - available_ammo_space / total_ammo_space;
	}
};

template <class E>
ammunition_information calc_reloadable_ammo_info(const E& item) {
	ammunition_information out;

	const auto maybe_magazine_slot = item[slot_function::GUN_DETACHABLE_MAGAZINE];

	if (maybe_magazine_slot.alive() && maybe_magazine_slot.has_items()) {
		const auto mag = item.get_cosmos()[maybe_magazine_slot.get_items_inside()[0]];
		const auto ammo_depo = mag[slot_function::ITEM_DEPOSIT];

		ensure(ammo_depo->has_limited_space());

		out.total_charges += count_charges_in_deposit(mag);
		out.total_ammo_space += ammo_depo->space_available;
		out.available_ammo_space += ammo_depo.calc_local_space_available();
	}

	return out;
}

template <class E>
ammunition_information calc_ammo_info(const E& item) {
	auto out = calc_reloadable_ammo_info(item);

	auto count_for = [&](const auto slot) {
		if (slot.alive()) {
			ensure(slot->has_limited_space());

			out.total_charges += count_charges_inside(slot);
			out.total_ammo_space += slot->space_available;
			out.available_ammo_space += slot.calc_local_space_available();
		}
	};

	const auto chamber = item[slot_function::GUN_CHAMBER];
	const auto chamber_mag = item[slot_function::GUN_CHAMBER_MAGAZINE];

	count_for(chamber);

	if (chamber_mag && chamber) {
		if (const auto gun = item.template find<invariants::gun>()) {
			if (!gun->allow_charge_in_chamber_magazine_when_chamber_loaded) {
				out.available_ammo_space -= chamber_mag.calc_space_occupied_by_children();
				out.total_charges += count_charges_inside(chamber_mag);
			}
			else {
				count_for(chamber_mag);
			}
		}
	}

	return out;
}
