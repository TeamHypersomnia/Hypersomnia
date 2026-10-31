#pragma once
#include "game/cosmos/cosmos.h"
#include "game/detail/gun/missile_multipliers.h"
#include "game/components/sender_component.h"
#include "game/components/cartridge_component.h"
#include "game/modes/detail/flavour_getters.h"

/*
	A Missile node (editor_missile_node, for test maps) fires its round as if from the gun whose charge it is:
	that gun as the sender, and its multipliers - so that what it does to a character (damage, headshots,
	how long a corpse stands, whether it falls at once, the size of the holes) is what a shot of that gun does.

	The first gun found shooting this round is taken (some rounds are shared by guns).
	A round no gun shoots stays a gunless missile.
*/

inline entity_flavour_id find_gun_shooting_round(const cosmos& cosm, const entity_flavour_id round) {
	auto round_of_charge = [&](const item_flavour_id charge) {
		auto result = entity_flavour_id();

		if (charge.is_set()) {
			cosm.on_flavour(charge, [&](const auto& typed_charge) {
				if (const auto cartridge = typed_charge.template find<invariants::cartridge>()) {
					result = entity_flavour_id(cartridge->round_flavour);
				}
			});
		}

		return result;
	};

	auto found = entity_flavour_id();

	cosm.for_each_flavour_having<invariants::gun>([&](const auto& id, const auto&) {
		if (found.is_set()) {
			return;
		}

		const auto gun = item_flavour_id(id);

		const auto magazine = ::get_allowed_flavour_of(cosm, gun, slot_function::GUN_DETACHABLE_MAGAZINE);

		const item_flavour_id charges[] = {
			::get_allowed_flavour_of(cosm, gun, slot_function::GUN_CHAMBER),
			::get_allowed_flavour_of(cosm, gun, slot_function::GUN_CHAMBER_MAGAZINE),
			magazine.is_set() ? ::get_allowed_flavour_of_deposit(cosm, magazine) : item_flavour_id()
		};

		for (const auto& charge : charges) {
			if (round_of_charge(charge) == round) {
				found = entity_flavour_id(id);
				return;
			}
		}
	});

	return found;
}

template <class R>
void set_missile_node_shot_by_its_gun(const cosmos& cosm, const R& round_entity) {
	const auto gun = ::find_gun_shooting_round(cosm, entity_flavour_id(round_entity.get_flavour_id()));

	if (!gun.is_set()) {
		return;
	}

	cosm.on_flavour(gun, [&](const auto& typed_gun) {
		if (const auto gun_def = typed_gun.template find<invariants::gun>()) {
			::set_missile_multipliers_of_gun(round_entity.template get<components::missile>(), *gun_def);
		}
	});

	round_entity.template get<components::sender>().direct_sender_flavour = gun;
}
