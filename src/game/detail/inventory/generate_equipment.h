#pragma once
#include "game/detail/inventory/requested_equipment.h"
#include "game/enums/slot_function.h"
#include "game/cosmos/just_create_entity.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/entity_handle_mixins/find_target_slot_for.hpp"

template <class E>
void requested_equipment::generate_for(
	const E& character, 
	const logic_step step
) const {
	static constexpr bool to_the_ground = std::is_same_v<E, transformr>;

	const auto& eq = *this;
	auto& cosm = step.get_cosmos();

	auto transfer = [&step](const auto from, const auto to) {
		if (to.dead()) {
			return;
		}

		if (const auto tr = to.get_container().find_logic_transform()) {
			from.set_logic_transform(*tr);
		}

		auto request = item_slot_transfer_request::standard(from, to);
		request.params.bypass_mounting_requirements = true;
		perform_transfer(request, step);
	};

	auto make_wearable = [&](const item_flavour_id& from, const slot_function slot) {
		if (!from.is_set()) {
			return;
		}

		if constexpr (!to_the_ground) {
			if (const auto target_slot = character[slot]; target_slot.is_empty_slot()) {
				const auto new_wearable = just_create_entity(cosm, from);
				transfer(new_wearable, target_slot);
			}
		}
		else {
			just_create_entity(cosm, from);
			(void)slot;
		}
	};

	make_wearable(eq.back_wearable, slot_function::BACK);
	make_wearable(eq.belt_wearable, slot_function::BELT);
	make_wearable(eq.personal_deposit_wearable, slot_function::PERSONAL_DEPOSIT);
	make_wearable(eq.shoulder_wearable, slot_function::SHOULDER);

	auto get_holstering_slot = [&](const auto& for_new_item) {
		if constexpr (to_the_ground) {
			return cosm[inventory_slot_id()];
		}
		else {
			return character.find_holstering_slot_for(for_new_item);
		}
	};

	const auto character_transform = [&]() {
		if constexpr (to_the_ground) {
			return character;
		}
		else {
			return character.get_logic_transform();	
		}
	}();

	if (eq.weapon.is_set()) {
		const auto weapon = just_create_entity(cosm, eq.weapon);

		/* So that the effect transform is valid */
		weapon.set_logic_transform(character_transform);

		if constexpr (!to_the_ground) {
			transfer(weapon, character.get_primary_hand());
		}

		auto make_mag = [&](const auto& mag_flavour) {
			if (mag_flavour.is_set()) {
				const auto magazine = just_create_entity(cosm, mag_flavour);
				const auto mag_deposit = magazine[slot_function::ITEM_DEPOSIT];

				const auto final_charge_flavour = [&]() {
					if (eq.non_standard_charge.is_set()) {
						return eq.non_standard_charge;
					}

					return mag_deposit->only_allow_flavour;
				}();

				if (final_charge_flavour.is_set()) {
					const auto c = just_create_entity(cosm, final_charge_flavour);
					c.set_charges(c.num_charges_fitting_in(mag_deposit));

					transfer(c, mag_deposit);
				}

				return magazine;
			}

			return cosm[entity_id()];
		};

		auto load_charge_to_chamber = [&](const auto& charge_flavour) {
			if (charge_flavour.is_set()) {
				const auto c = just_create_entity(cosm, charge_flavour);
				c.set_charges(1);

				if (const auto chamber = weapon[slot_function::GUN_CHAMBER]) {
					transfer(c, chamber);
				}
			}
		};

		const auto magazine_slot = weapon[slot_function::GUN_DETACHABLE_MAGAZINE];

		if (magazine_slot) {
			const auto final_mag_flavour = [&]() {
				if (eq.non_standard_mag.is_set()) {
					return eq.non_standard_mag;
				}

				return magazine_slot->only_allow_flavour;
			}();

			if (const auto new_mag = make_mag(final_mag_flavour)) {
				transfer(new_mag, magazine_slot);

				if (const auto loaded_charge = new_mag[slot_function::ITEM_DEPOSIT].get_item_if_any()) {
					load_charge_to_chamber(loaded_charge.get_flavour_id());
				}

				auto n = eq.num_spare_ammo_pieces;

				while (n--) {
					const auto spare_mag = make_mag(final_mag_flavour);
					const auto target_slot = get_holstering_slot(spare_mag);

					transfer(spare_mag, target_slot);
				}
			}
		}
	}

	for (const auto& it : eq.other_equipment) {
		auto n = it.first;
		const auto& f = it.second;

		while (n--) {
			if (const auto new_item = just_create_entity(cosm, f)) {
				transfer(new_item, get_holstering_slot(new_item));
			}
		}
	}
}
