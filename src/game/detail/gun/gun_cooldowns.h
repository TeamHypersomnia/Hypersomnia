#pragma once

template <class T>
real32 calc_current_chambering_duration(const T& it) {
	const auto& gun_def = it.template get<invariants::gun>();
	const auto& gun = it.template get<components::gun>();
	const auto chamber_slot = it[slot_function::GUN_CHAMBER];

	if (chamber_slot && chamber_slot->is_mounted_slot()) {
		return 
			gun.special_state == gun_special_state::AFTER_BURST
			? gun_def.after_burst_chambering_ms
			: chamber_slot->mounting_duration_ms
		;
	}
	
	return 0.f;
}
