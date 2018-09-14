#pragma once
#include <array>
#include <functional>

#include "augs/misc/timing/delta.h"
#include "augs/misc/randomization.h"
#include "augs/math/camera_cone.h"

#include "view/viewables/all_viewables_declaration.h"
#include "view/audiovisual_state/systems/audiovisual_cache_common.h"

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

struct frame_profiler;
struct draw_renderable_input;
struct randomization;

struct light_system_input {
	augs::renderer& renderer;
	frame_profiler& profiler;
	additive_time_scope& total_layer_scope;

	const cosmos& cosm;
	std::array<float, 16> projection_matrix;
	const augs::graphics::fbo& light_fbo;
	const augs::graphics::shader_program& light_shader;
	const augs::graphics::shader_program& standard_shader;
	std::function<void()> neon_callback;
	const camera_cone cone;
	std::optional<entity_id> fog_of_war_character;
	const float camera_query_mult;
	const particles_simulation_system& particles;
	const plain_animations_pool& plain_animations;
	const visible_entities& visible;

	const draw_renderable_input& drawing_in;
};

struct light_system {
	struct cache {
		std::array<float, 10> all_variation_values = {};
	};

	audiovisual_cache_map<cache> per_entity_cache;

	void reserve_caches_for_entities(const size_t);
	void clear();

	void advance_attenuation_variations(
		randomization& rng,
		const cosmos& cosmos,
		const augs::delta
	);

	void render_all_lights(const light_system_input) const;
};