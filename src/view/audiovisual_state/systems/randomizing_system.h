#pragma once
#include "augs/misc/timing/delta.h"
#include "augs/drawing/sprite.h"
#include "augs/misc/randomization.h"
#include "view/audiovisual_state/systems/audiovisual_cache_common.h"
#include "augs/graphics/rgba.h"

namespace augs {
	class renderer;

	namespace graphics {
		class fbo;
		class shader_program;
	}
}

struct randomizing_system {
	struct walk_cache {
		real32 walk_state = 0.5f;
	};

	std::array<walk_cache, 10> random_walks;

	void reserve_caches_for_entities(const size_t);
	void clear();

	void advance(augs::delta);

	float get_random_walk_mult(
		const entity_id id,
		const intensity_vibration_input& in
	) const;

	randomization rng;
};