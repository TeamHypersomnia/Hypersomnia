#pragma once
#include <array>
#include <functional>

#include "augs/misc/timing/delta.h"
#include "augs/misc/randomization.h"
#include "augs/math/camera_cone.h"

#include "game/stateless_systems/visibility_system.h"

#include "view/viewables/all_viewables_declaration.h"
#include "view/audiovisual_state/systems/audiovisual_cache_common.h"

#include "game/detail/visible_entities.h"
#include "application/performance_settings.h"

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

	const cosmos& cosm;
	std::array<float, 16> projection_matrix;
	const augs::graphics::fbo& light_fbo;
	const augs::graphics::shader_program& light_shader;
	const augs::graphics::shader_program& textured_light_shader;
	const augs::graphics::shader_program& standard_shader;
	std::function<void(bool)> neon_occlusion_callback;
	std::function<void()> neon_callback;
	std::function<void()> write_fow_to_stencil;
	const camera_cone cone;
	std::optional<entity_id> fog_of_war_character;
	const camera_cone queried_cone;
	const visible_entities& visible;

	const augs::atlas_entry cast_highlight_tex;
	std::function<draw_renderable_input()> make_drawing_in;
	const performance_settings& perf_settings;
	const std::vector<visibility_request>& requests;
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
		const cosmos& cosm,
		const augs::delta
	);

	void render_all_lights(const light_system_input) const;
	void gather_vis_requests(
		const cosmos& cosm,
		const interpolation_system& interp,
		ltrb queried_camera_aabb
	) const;
};