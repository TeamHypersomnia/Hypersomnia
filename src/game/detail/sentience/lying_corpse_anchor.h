#pragma once
#include <optional>
#include "augs/math/transform.h"
#include "game/enums/lying_corpse_anchor.h"
#include "game/components/sentience_component.h"
#include "game/detail/sentience/lying_corpse_flip.h"

/*
	Where on a character's lying corpse an effect that chases the character goes once it lies:
	its neck, facing away from the body (the stream after a headshot), or its middle at the effect's own offset
	(the fire of an ignited corpse). Found anew every time from the character - it never depends on the lying corpse's
	entity, which may differ between the predicted and the referential worlds.
*/

template <class H, class I>
std::optional<transformr> find_on_lying_corpse(
	const H& character,
	const lying_corpse_anchor anchor,
	const transformr offset,
	I& interp
) {
	if (anchor == lying_corpse_anchor::NONE) {
		return std::nullopt;
	}

	if (const auto sentience = character.template find<components::sentience>()) {
		const auto& cosm = character.get_cosmos();

		if (const auto lying_corpse = cosm[sentience->detached.lying_corpse]) {
			if (const auto corpse_transform = lying_corpse.find_viewing_transform(interp)) {
				if (anchor == lying_corpse_anchor::MIDDLE) {
					return *corpse_transform * offset;
				}

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
