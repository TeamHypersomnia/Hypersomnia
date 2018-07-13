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

	augs::delta last_frame_delta = augs::delta::zero;

	audiovisual_cache_map<walk_cache> neon_intensity_walks;

	void reserve_caches_for_entities(const size_t);
	void clear();

	float advance_and_get_neon_mult(
		const entity_id id,
		const intensity_vibration_input& in
	);

	randomization rng;
};