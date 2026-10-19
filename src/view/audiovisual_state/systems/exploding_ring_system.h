#pragma once
#include <cstddef>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "augs/misc/bound.h"
#include "augs/misc/timing/delta.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/math/camera_cone.h"

#include "augs/drawing/sprite.h"
#include "augs/drawing/drawing.h"

#include "game/messages/exploding_ring_input.h"
#include "game/components/transform_component.h"

#include "view/view_container_sizes.h"

class particles_simulation_system;
struct randomization;
struct common_assets;
struct explosions_settings;
struct continuous_rings_input;
struct particles_emission;
class cosmos;

class exploding_ring_system {
public:
	struct ring {
		exploding_ring_input in;
		double time_of_occurence_seconds = 0.0;

		/*
			Cached from the client's settings every advance, for the drawing.
			Before the first advance, the light stays at the palette's start.
		*/
		float palette_step_ms = std::numeric_limits<float>::max();
		float light_brightness_mult = 1.f;
	};

	double global_time_seconds = 0.0;

	/*
		The visibility last calculated for a purely visual explosion (see pending_visibility_radius) -
		reused by all its rings and jets, which come in the same advance.
	*/

	struct cached_visibility {
		vec2 center;
		entity_id subject;
		float radius = -1.f;
		double time_seconds = -1.0;
		messages::visibility_information_response response;
	};

	cached_visibility last_visibility;

	/*
		The portals with pixel particles in view this frame (see spawn_portal_particles) -
		caches kept between the frames so as not to allocate.
	*/

	struct visible_portal {
		entity_id id;
		vec2 center;
		float radius = 0.f;
		const continuous_rings_input* rings = nullptr;
		float count = 0.f;
		int alive = 0;
	};

	std::vector<visible_portal> visible_portals;
	std::unordered_map<entity_id, std::size_t> visible_portal_indices;
	std::unordered_set<entity_id> portals_visible_last_frame;

	void spawn_portal_particles(
		const cosmos& cosm,
		randomization& rng,
		const ltrb queried_camera_aabb,
		const explosions_settings& settings,
		const particles_emission& emission,
		particles_simulation_system& particles,
		const float amount_mult,
		const float dt_secs
	);

	augs::constant_size_vector<ring, MAX_EXPLODING_RINGS> rings;

	template <class C>
	void acquire_new_rings(const C& rings);
	
	template <class I>
	void acquire_new_ring(I&&);

	void advance(
		const cosmos& cosm,
		randomization& rng,
		const camera_cone queried_cone,
		const common_assets& common,
		const particle_effects_map&,
		const augs::delta dt,
		const explosions_settings& settings,
		const float portal_particle_amount,
		const bool portal_particles_enabled,
		particles_simulation_system& particles_output_for_effects
	);

	void draw_rings(
		const augs::drawer_with_default output,
		augs::special_buffer& specials,
		const camera_cone queried_cone,
		const camera_cone actual_cone
	) const;

	void draw_portal_swirls(
		const cosmos& cosm,
		const augs::drawer_with_default output,
		augs::special_buffer& specials,
		const camera_cone queried_cone,
		const camera_cone actual_cone
	) const;

	void draw_continuous_rings(
		const cosmos& cosm,
		const augs::drawer_with_default output,
		augs::special_buffer& specials,
		const camera_cone queried_cone,
		const camera_cone actual_cone
	) const;

	void draw_highlights_of_continuous_rings(
		const cosmos& cosm,
		const augs::drawer output,
		const augs::atlas_entry highlight_tex,
		const camera_cone queried_cone
	) const;

	void draw_highlights_of_explosions(
		const augs::drawer output,
		const augs::atlas_entry highlight_tex,
		const camera_cone queried_cone
	) const;

	void reserve_caches_for_entities(const size_t) const {}
	void clear();
};