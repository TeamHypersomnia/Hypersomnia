#pragma once
#include "application/setups/editor/editor_folder.h"

template <class C>
auto editor_folder::make_player_advance_input(const C& with_callbacks) {
	return player_advance_input(
		mode_vars,
		work->world,
		with_callbacks
	);
}
