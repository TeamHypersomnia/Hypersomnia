#pragma once
#include "game/detail/inventory/requested_equipment.h"
#include "game/enums/slot_function.h"
#include "game/cosmos/just_create_entity.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/entity_handle_mixins/find_target_slot_for.hpp"
#include "game/modes/detail/item_purchase_logic.hpp"

template <class E>
entity_id requested_equipment::generate_for(
	const E& character, 
	const logic_step step
) const {
	static constexpr bool to_the_ground = std::is_same_v<E, transformr>;

	const auto& eq = *this;
	auto& cosm = step.get_cosmos();

	auto transfer = [&step](const auto from, const auto to, const bool play_effects = true) {
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
		request.params.play_transfer_sounds = play_effects;
		request.params.play_transfer_particles = play_effects;

		perform_transfer(request, step);
	};

	auto make_wearable = [&, character](const auto& from, const slot_function slot) {
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
			(void)character;
		}
	};

	make_wearable(eq.back_wearable, slot_function::BACK);
	make_wearable(eq.belt_wearable, slot_function::BELT);
	make_wearable(eq.personal_deposit_wearable, slot_function::PERSONAL_DEPOSIT);
	make_wearable(eq.shoulder_wearable, slot_function::SHOULDER);
	make_wearable(eq.armor_wearable, slot_function::TORSO_ARMOR);

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
		if (what.dead()) {
			return;
		}

		transfer(what, get_pickup_slot_for(what));
	};

	const auto character_transform = [&](auto&&...) {
		if constexpr (to_the_ground) {
			return character;
		}
		else {
			return character.get_logic_transform();	
		}
	}();

	auto make_ammo_piece = [&](const auto& flavour) {
		if (flavour.is_set()) {
			if (const auto piece = just_create_entity(cosm, flavour)) {
				if (const auto mag_deposit = piece[slot_function::ITEM_DEPOSIT]) {
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
				}

				return piece;
			}
		}

		return cosm[entity_id()];
	};

	auto ammo_pieces_to_generate_left = eq.num_given_ammo_pieces;

	auto generate_spares = [&](const item_flavour_id& f) {
		while (ammo_pieces_to_generate_left-- > 0) {
			pickup(make_ammo_piece(f));
		}
	};

	entity_id result_weapon;

	if (eq.weapon.is_set()) {
		if (const auto weapon = just_create_entity(cosm, eq.weapon)) {
			result_weapon = weapon.get_id();

			/* So that the effect transform is valid */
			weapon.set_logic_transform(character_transform);

			if constexpr(!to_the_ground) {
				pickup(weapon);
			}

			const auto chamber_slot = weapon[slot_function::GUN_CHAMBER];
			const auto chamber_mag_slot = weapon[slot_function::GUN_CHAMBER_MAGAZINE];

			auto create_charge_in_chamber = [&](const auto& charge_flavour) {
				if (charge_flavour.is_set()) {
					if (chamber_slot) {
						if (const auto c = just_create_entity(cosm, charge_flavour)) {
							c.set_charges(1);

							transfer(c, chamber_slot, false);

							if (chamber_mag_slot) {
								if (const auto num_fitting = c.num_charges_fitting_in(chamber_mag_slot); num_fitting > 0) {
									if (const auto cm = just_create_entity(cosm, charge_flavour)) {
										cm.set_charges(num_fitting);

										transfer(cm, chamber_mag_slot, false);
									}
								}
							}
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

				if (ammo_pieces_to_generate_left < 0) {
					ammo_pieces_to_generate_left = weapon.template get<invariants::item>().gratis_ammo_pieces_with_first;
				}

				if (ammo_pieces_to_generate_left > 0) {
					if (const auto new_mag = make_ammo_piece(final_mag_flavour)) {
						transfer(new_mag, magazine_slot, false);
						--ammo_pieces_to_generate_left;

						if (const auto loaded_charge = new_mag[slot_function::ITEM_DEPOSIT].get_item_if_any()) {
							create_charge_in_chamber(loaded_charge.get_flavour_id());
						}

						generate_spares(final_mag_flavour);
					}
				}
			}
			else if (chamber_slot) {
				const auto final_charge_flavour = [&]() {
					if (eq.non_standard_mag.is_set()) {
						return eq.non_standard_charge;
					}

					return chamber_slot->only_allow_flavour;
				}();

				if (ammo_pieces_to_generate_left < 0) {
					ammo_pieces_to_generate_left = weapon.template get<invariants::item>().gratis_ammo_pieces_with_first;
				}

				if (ammo_pieces_to_generate_left > 0) {
					create_charge_in_chamber(final_charge_flavour);
					--ammo_pieces_to_generate_left;

					generate_spares(final_charge_flavour);
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

	return result_weapon;
}
