#pragma once
#include <array>
#include <functional>

#include "augs/misc/randomization.h"
#include "augs/misc/delta.h"

#include "augs/math/camera_cone.h"

#include "game/assets/assets_declarations.h"
#include "game/detail/visible_entities.h"

class interpolation_system;
class particles_simulation_system;

namespace augs {
	class renderer;

	namespace graphics {
		class fbo;
		class shader_program;
	}
}

struct light_system_input {
	augs::renderer& renderer;
	const cosmos& cosm;
	std::array<float, 16> projection_matrix;
	const augs::graphics::fbo& light_fbo;
	const augs::graphics::shader_program& light_shader;
	const augs::graphics::shader_program& standard_shader;
	std::function<void()> neon_callback;
	const camera_cone camera;
	const interpolation_system& interpolation;
	const particles_simulation_system& particles;
	const visible_entities::per_layer_type& visible_per_layer;
	const game_images_in_atlas& game_images;
};

class light_system {
	double global_time_seconds = 0.0;

public:
	struct cache {
		std::array<float, 10> all_variation_values;
		cache();
	};

	std::vector<cache> per_entity_cache;

	randomization rng;

	void reserve_caches_for_entities(const size_t);

	void advance_attenuation_variations(
		const cosmos& cosmos,
		const augs::delta
	);

	void render_all_lights(const light_system_input) const;
};