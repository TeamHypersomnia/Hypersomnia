#pragma once
#include "game/detail/inventory/weapon_reloading.h"

inline bool reloading_context::significantly_different_from(const reloading_context& b) const {
	return 
		concerned_slot != b.concerned_slot 
		|| new_ammo_source != b.new_ammo_source 
	;
}

inline bool reloading_context::is_chambering() const {
	return !new_ammo_source.is_set() && !old_ammo_source.is_set();
}
