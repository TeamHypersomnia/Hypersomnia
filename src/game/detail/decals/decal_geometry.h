#pragma once
#include "augs/math/vec2.h"
#include "game/components/sprite_component.h"

/*
	The decal's actual on-screen size: the per-entity override if set,
	otherwise the flavour's sprite size.
*/
template <class E>
vec2 get_decal_size(const E& typed_decal) {
	if constexpr(std::remove_cvref_t<E>::template has<components::overridden_geo>()) {
		const auto& overridden_size = typed_decal.template get<components::overridden_geo>().get();

		if (overridden_size.is_enabled) {
			return vec2(overridden_size.value);
		}
	}

	return vec2(typed_decal.template get<invariants::sprite>().size);
}
