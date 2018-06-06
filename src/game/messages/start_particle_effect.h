#pragma once
#include <optional>
#include "game/messages/message.h"
#include "augs/math/vec2.h"

#include "game/transcendental/entity_id.h"
#include "game/components/transform_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/assets/ids/asset_ids.h"

namespace messages {
	struct start_particle_effect {
		packaged_particle_effect payload;
	};

	struct stop_particle_effect {
		std::optional<entity_id> match_chased_subject;
		std::optional<vec2> match_orbit_offset;
		std::optional<assets::particle_effect_id> match_effect_id;
	};
}
