#pragma once
#include "game/detail/inventory/calc_reloading_context.hpp"
#include "game/detail/calc_ammo_info.hpp"

template <class E, class C>
bool is_ammo_depleted(const E& gun_entity, const C& capability) {
	const auto ammo_info = calc_ammo_info(gun_entity);

	if (ammo_info.total_ammo_space > 0 && ammo_info.total_charges == 0) {
		return calc_reloading_context_for(capability, gun_entity) == std::nullopt;
	}

	return false;
}
