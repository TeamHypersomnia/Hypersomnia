#pragma once
#include "game/detail/inventory/requested_equipment.h"
#include "game/enums/slot_function.h"
#include "game/cosmos/just_create_entity.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/entity_handle_mixins/find_target_slot_for.hpp"
#include "game/modes/detail/item_purchase_logic.hpp"

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

		if (from.dead()) {
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

	auto get_pickup_slot_for = [&](const auto& new_item) {
		if constexpr (to_the_ground) {
			return cosm[inventory_slot_id()];
		}
		else {
			const auto slot = character.find_pickup_target_slot_for(new_item);
			// LOG_NVPS(new_item, slot);
			return slot;
		}
	};

	auto pickup = [&](const auto& what) {
		transfer(what, get_pickup_slot_for(what));
	};

	const auto character_transform = [&]() {
		if constexpr (to_the_ground) {
			return character;
		}
		else {
			return character.get_logic_transform();	
		}
	}();

	auto make_mag = [&](const auto& mag_flavour) {
		if (mag_flavour.is_set() && is_magazine_like(cosm, mag_flavour)) {
			if (const auto magazine = just_create_entity(cosm, mag_flavour)) {
				const auto mag_deposit = magazine[slot_function::ITEM_DEPOSIT];

				const auto final_charge_flavour = [&]() {
					if (eq.non_standard_charge.is_set()) {
						return eq.non_standard_charge;
					}

					return mag_deposit->only_allow_flavour;
				}();

				if (final_charge_flavour.is_set()) {
					if (const auto c = just_create_entity(cosm, final_charge_flavour)) {
						c.set_charges(c.num_charges_fitting_in(mag_deposit));

						transfer(c, mag_deposit);
					}
				}

				return magazine;
			}
		}

		return cosm[entity_id()];
	};

	auto n = eq.num_given_ammo_pieces;

	auto generate_spares = [&](const item_flavour_id& f) {
		while (n-- > 0) {
			pickup(make_mag(f));
		}
	};

	if (eq.weapon.is_set()) {
		if (const auto weapon = just_create_entity(cosm, eq.weapon)) {
			/* So that the effect transform is valid */
			weapon.set_logic_transform(character_transform);

			if constexpr (!to_the_ground) {
				pickup(weapon);
			}

			auto load_charge_to_chamber = [&](const auto& charge_flavour) {
				if (charge_flavour.is_set()) {
					if (const auto c = just_create_entity(cosm, charge_flavour)) {
						c.set_charges(1);

						if (const auto chamber = weapon[slot_function::GUN_CHAMBER]) {
							transfer(c, chamber);
						}
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

				if (n < 0) {
					n = weapon.template get<invariants::item>().gratis_ammo_pieces_with_first;
				}

				if (n > 0) {
					if (const auto new_mag = make_mag(final_mag_flavour)) {
						transfer(new_mag, magazine_slot);
						--n;

						if (const auto loaded_charge = new_mag[slot_function::ITEM_DEPOSIT].get_item_if_any()) {
							load_charge_to_chamber(loaded_charge.get_flavour_id());
						}

						generate_spares(final_mag_flavour);
					}
				}
			}
		}
	}
	else {
		if (const auto& f = eq.non_standard_mag; f.is_set()) {
			generate_spares(f);
		}
	}

	for (const auto& it : eq.other_equipment) {
		auto n = it.first;
		const auto& f = it.second;

		while (n--) {
			pickup(just_create_entity(cosm, f));
		}
	}

	if constexpr(!to_the_ground) {
		if (const auto sentience = character.template find<components::sentience>()) {
			for (const auto& s : eq.spells_to_give) {
				if (s) {
					const auto i = index_in(eq.spells_to_give, s);

					sentience->learnt_spells[i] = true;
				}
			}
		}
	}
}
