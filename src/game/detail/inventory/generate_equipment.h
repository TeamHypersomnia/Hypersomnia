#pragma once
#include "game/detail/inventory/requested_equipment.h"
#include "game/enums/slot_function.h"
#include "game/cosmos/just_create_entity.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"

template <class E>
void generate_equipment(const requested_equipment& eq, const E& character, const logic_step step) {
	auto& cosm = character.get_cosmos();

	const auto character_transform = character.get_logic_transform();

	auto transfer = [&step](const auto from, const auto to) {
		perform_transfer(item_slot_transfer_request::standard(from, to), step);
	};

	if (eq.weapon.is_set()) {
		const auto weapon = just_create_entity(cosm, eq.weapon);

		/* So that the effect transform is valid */
		weapon.set_logic_transform(character_transform);

		const auto magazine_slot = weapon[slot_function::GUN_DETACHABLE_MAGAZINE];

		if (eq.magazine.is_set()) {
			const auto magazine = just_create_entity(cosm, eq.magazine);

			transfer(magazine, magazine_slot);

			if (eq.charge.is_set()) {
				{
					const auto c = just_create_entity(cosm, eq.charge);
					c.set_charges(1);

					if (const auto chamber = weapon[slot_function::GUN_CHAMBER]) {
						transfer(c, chamber);
					}
				}

				const auto c = just_create_entity(cosm, eq.charge);
				const auto mag_deposit = magazine[slot_function::ITEM_DEPOSIT];
				c.set_charges(c.num_charges_fitting_in(mag_deposit));

				transfer(c, mag_deposit);
			}
		}

		transfer(weapon, character.get_primary_hand());
	}

	if (eq.backpack.is_set()) {
		const auto b = just_create_entity(cosm, eq.backpack);

		transfer(b, character[slot_function::SHOULDER]);
	}
}
