#pragma once
#include <array>
#include <functional>

#include "augs/misc/timing/delta.h"

#include "augs/math/camera_cone.h"

#include "view/viewables/all_viewables_declarations.h"
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

struct light_system_input {
	augs::renderer& renderer;
	frame_profiler& profiler;

	const cosmos& cosm;
	std::array<float, 16> projection_matrix;
	const augs::graphics::fbo& light_fbo;
	const augs::graphics::shader_program& light_shader;
	const augs::graphics::shader_program& standard_shader;
	std::function<void()> neon_callback;
	const camera_cone camera;
	const vec2 screen_size;
	const particles_simulation_system& particles;
	const plain_animations_pool& plain_animations;
	const visible_entities::per_layer_type& visible_per_layer;

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
		const cosmos& cosmos,
		const augs::delta
	);

	void render_all_lights(const light_system_input) const;
};