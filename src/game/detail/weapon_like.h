#pragma once

template <class E>
bool is_weapon_like(const E& typed_handle) {
	return 
		typed_handle.template has<components::gun>() 
		|| typed_handle.template has<components::hand_fuse>()
		|| typed_handle.template has<components::melee>()
	;
}

