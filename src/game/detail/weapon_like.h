#pragma once

template <class E>
bool is_weapon_like(const E& typed_handle) {
	return 
		typed_handle.template has<components::gun>() 
		|| typed_handle.template has<components::hand_fuse>()
		|| typed_handle.template has<components::melee>()
	;
}

template <class E>
bool is_armor_like(const E& typed_handle) {
	return 
		typed_handle.template has<invariants::item>() 
		&& typed_handle.template get<invariants::item>().categories_for_slot_compatibility.test(item_category::TORSO_ARMOR)
	;
}

