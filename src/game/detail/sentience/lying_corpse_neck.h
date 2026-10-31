#pragma once
#include <optional>
#include "augs/math/transform.h"
#include "game/components/sentience_component.h"
#include "game/detail/sentience/lying_corpse_flip.h"

/*
	The neck of a character's lying corpse, if it has one, facing away from the body - for what spurts from it
	(the stream after a headshot) to move onto the lying corpse by itself. Found anew every time from the character:
	it never depends on the lying corpse's entity, which may differ between the predicted and the referential worlds.
*/

template <class H, class I>
std::optional<transformr> find_lying_corpse_neck(const H& character, I& interp) {
	if (const auto sentience = character.template find<components::sentience>()) {
		const auto& cosm = character.get_cosmos();

		if (const auto lying_corpse = cosm[sentience->detached.lying_corpse]) {
			if (const auto corpse_transform = lying_corpse.find_viewing_transform(interp)) {
				auto neck = cosm.get_logical_assets().get_offsets(lying_corpse.get_image_id()).torso.head;

				if (::is_lying_corpse_flipped(lying_corpse)) {
					neck.flip_vertically();
				}

				return *corpse_transform * transformr(vec2(neck.pos), neck.rotation + 180.f);
			}
		}
	}

	return std::nullopt;
}
