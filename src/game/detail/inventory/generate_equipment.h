#pragma once
#include "game/detail/inventory/requested_equipment.h"
#include "game/enums/slot_function.h"
#include "game/cosmos/just_create_entity.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/entity_handle_mixins/find_target_slot_for.hpp"
#include "game/modes/detail/item_purchase_logic.hpp"
#include "game/modes/detail/owner_meta_logic.h"
#include "game/cosmos/just_create_entity_functional.h"
#include "game/cosmos/allocate_new_entity_access.h"

template <class E>
entity_id requested_equipment::generate_for(
	allocate_new_entity_access access,
	const E& character, 
	const logic_step step,
	int max_effects_played
) const {
	return generate_for_impl(access, character, step.get_cosmos(), [&](auto callback){ callback(step); }, max_effects_played);
}

template <class E, class F>
entity_id requested_equipment::generate_for_impl(
	allocate_new_entity_access access,
	const E& character, 
	cosmos& cosm,
	F&& on_step,
	int max_effects_played
) const {
	static constexpr bool to_the_ground = std::is_same_v<E, transformr>;

	const auto& eq = *this;

	auto make_owned_item = [&](const auto& flavour) {
		const auto new_entity = just_create_entity(access, cosm, flavour, [&](const entity_handle h){ 
			if constexpr(to_the_ground) {
				h.set_logic_transform(character);
			}
		}, [](entity_handle){});

		if constexpr (!to_the_ground) {
			::set_original_owner(new_entity, character);
		}

		return new_entity;
	};

	auto recoils = perform_recoils;

	auto transfer = [recoils, &cosm, &on_step, &max_effects_played](const auto from, const auto to, const bool play_effects = true) {
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
		request.params.play_transfer_sounds = max_effects_played > 0 && play_effects;
		request.params.play_transfer_particles = max_effects_played > 0 && play_effects;
		request.params.perform_recoils = recoils;

		const auto result = perform_transfer_no_step(request, cosm);

		on_step([&result](auto&& step) {
			result.notify(step);
			result.play_effects(step);
		});

		if (play_effects) {
			--max_effects_played;
		}
	};

	auto make_wearable = [&, character](const auto& from, const slot_function slot) {
		if (!from.is_set()) {
			return;
		}

		if constexpr (!to_the_ground) {
			if (const auto target_slot = character[slot]; target_slot.is_empty_slot()) {
				const auto new_wearable = make_owned_item(from);
				transfer(new_wearable, target_slot);
			}
		}
		else {
			make_owned_item(from);
			(void)slot;
			(void)character;
		}
	};

	make_wearable(eq.back_wearable, slot_function::BACK);
#if 0
	make_wearable(eq.over_back_wearable, slot_function::OVER_BACK);
#endif
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

	const auto character_transform = [&](auto&&...) {
		if constexpr (to_the_ground) {
			return character;
		}
		else {
			return character.get_logic_transform();	
		}
	}();

	auto pickup = [&](const auto& what) {
		if (what.dead()) {
			return;
		}

		if constexpr (to_the_ground) {
			what.set_logic_transform(character_transform);
		}
		else {
			transfer(what, get_pickup_slot_for(what));
		}
	};

	auto make_ammo_piece = [&](const auto& flavour) {
		if (flavour.is_set()) {
			if (const auto piece = make_owned_item(flavour)) {
				if (const auto mag_deposit = piece[slot_function::ITEM_DEPOSIT]) {
					const auto final_charge_flavour = [&]() {
						if (eq.non_standard_charge.is_set()) {
							return eq.non_standard_charge;
						}

						return mag_deposit->only_allow_flavour;
					}();

					if (final_charge_flavour.is_set()) {
						if (const auto c = make_owned_item(final_charge_flavour)) {
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

	/* Min for security */
	auto ammo_pieces_to_generate_left = std::min(10, eq.num_given_ammo_pieces);

	auto generate_spares = [&](const item_flavour_id& f) {
		entity_id piece;

		while (ammo_pieces_to_generate_left-- > 0) {
			auto next = make_ammo_piece(f);
			piece = next.get_id();
			pickup(next);
		}

		return piece;
	};

	entity_id result_weapon;

	if (eq.weapon.is_set()) {
		if (const auto weapon = make_owned_item(eq.weapon)) {
			result_weapon = weapon.get_id();

			/* So that the effect transform is valid */
			weapon.set_logic_transform(character_transform);

			const auto chamber_slot = weapon[slot_function::GUN_CHAMBER];
			const auto chamber_mag_slot = weapon[slot_function::GUN_CHAMBER_MAGAZINE];

			auto create_charge_in_chamber = [&](const auto& charge_flavour) {
				if (charge_flavour.is_set()) {
					if (chamber_slot) {
						if (const auto c = make_owned_item(charge_flavour)) {
							c.set_charges(1);

							transfer(c, chamber_slot, false);

							if (chamber_mag_slot) {
								if (const auto num_fitting = c.num_charges_fitting_in(chamber_mag_slot); num_fitting > 0) {
									if (const auto cm = make_owned_item(charge_flavour)) {
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

			if constexpr(!to_the_ground) {
				pickup(weapon);
			}
		}
	}
	else {
		if (const auto& f = eq.non_standard_mag; f.is_set()) {
			result_weapon =	generate_spares(f);
		}
	}

	for (const auto& it : eq.other_equipment) {
		auto n = it.first;
		const auto& f = it.second;

		while (n--) {
			pickup(make_owned_item(f));
		}
	}

	if constexpr(!to_the_ground) {
		if (const auto sentience = character.template find<components::sentience>()) {
			if (haste_time > 0.f) {
				auto& haste = sentience->template get<haste_perk_instance>();
				haste.timing.set_for_duration(haste_time * 1000, cosm.get_timestamp()); 
				haste.is_greater = true;
			}

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

inline bool requested_equipment::has_weapon(entity_flavour_id id) const {
	if (id == weapon || id == back_wearable || id == belt_wearable || id == personal_deposit_wearable || id == shoulder_wearable || id == armor_wearable) {
		return true;
	}

	for (auto o : other_equipment) {
		if (o.second == id) {
			return true;
		}
	}

	return false;
}
