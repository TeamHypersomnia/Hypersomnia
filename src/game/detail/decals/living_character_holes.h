#pragma once
#include "game/components/decal_component.h"
#include "game/components/sentience_component.h"

/*
	The holes of a character still alive are not drawn (see owned_decals.h), so they are kept out of
	the tree of non-physical objects and don't follow the character every step - only their place on its body
	is kept (attachment_offset). They go into the tree once it dies - see reveal_holes_of_the_dead.
*/

template <class E>
bool is_hole_of_living_character(const E& decal_handle) {
	if constexpr(E::template has<components::decal>()) {
		if (!decal_handle.template get<invariants::decal>().is_character_decal) {
			return false;
		}

		const auto& state = decal_handle.template get<components::decal>();

		if (const auto owner = decal_handle.get_cosmos()[state.attached_to]) {
			if (const auto sentience = owner.template find<components::sentience>()) {
				return !sentience->is_dead();
			}
		}
	}

	return false;
}
