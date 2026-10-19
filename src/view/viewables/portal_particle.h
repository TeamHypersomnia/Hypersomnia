#pragma once
#include <algorithm>
#include <limits>
#include "game/cosmos/entity_id.h"
#include "view/viewables/particle_types.h"
#include "view/viewables/particle_types.hpp"

/*
	A portal's pixel particle (see continuous_rings_particles) - for making new portal effects, in developer builds.

	It knows its portal, so that the particles of a portal out of view are dropped,
	and each portal's are counted against its budget (see portal_particles_spawner).

	Besides an explosion particle's coloring, it moves at a constant radial speed and spin,
	stopping at max_radius with stop_at_max_radius, and its alpha fades in and out up to max_alpha.
	With edge_fade_ms above 0, once past edge_fade_radius it fades out over edge_fade_ms instead of living out its lifetime,
	keeping the size and alpha it had then.
*/

struct portal_particle : explosion_particle {
	entity_id owner;

	bool stop_at_max_radius = false;
	float angular_vel = 0.f;

	float fade_in_ms = 0.f;
	float fade_out_ms = 0.f;
	float max_alpha = 1.f;

	float edge_fade_radius = std::numeric_limits<float>::max();
	float edge_fade_ms = 0.f;
	float edge_fade_start_ms = -1.f;
	float edge_fade_from_alpha = 1.f;
	float lifetime_before_edge_fade_ms = 0.f;

	bool is_edge_fading() const {
		return edge_fade_start_ms >= 0.f;
	}

	float calc_lifetime_alpha() const {
		const auto age_ms = sprite.current_lifetime_ms;
		auto result = max_alpha;

		if (fade_in_ms > 0.f) {
			result *= std::min(1.f, age_ms / fade_in_ms);
		}

		if (fade_out_ms > 0.f) {
			result *= std::clamp((sprite.max_lifetime_ms - age_ms) / fade_out_ms, 0.f, 1.f);
		}

		return result;
	}

	void begin_edge_fade() {
		edge_fade_from_alpha = calc_lifetime_alpha();
		edge_fade_start_ms = sprite.current_lifetime_ms;
		lifetime_before_edge_fade_ms = sprite.max_lifetime_ms;
		sprite.max_lifetime_ms = sprite.current_lifetime_ms + edge_fade_ms;
	}

	void update_sprite_color() {
		explosion_particle::update_sprite_color();

		const auto age_ms = sprite.current_lifetime_ms;

		const auto alpha = 
			is_edge_fading() ?
			edge_fade_from_alpha * std::clamp(1.f - (age_ms - edge_fade_start_ms) / edge_fade_ms, 0.f, 1.f) :
			calc_lifetime_alpha()
		;

		sprite.color.a = static_cast<rgba_channel>(255.f * alpha);
	}

	void integrate(const float dt) {
		sprite.current_lifetime_ms += dt * 1000;

		radius += radial_vel * dt;
		angle += angular_vel * dt;

		if (radius > max_radius && stop_at_max_radius) {
			radius = max_radius;
			radial_vel = 0.f;
		}

		if (radius < 0.f || radius > max_radius) {
			sprite.current_lifetime_ms = sprite.max_lifetime_ms;
		}

		if (edge_fade_ms > 0.f && !is_edge_fading() && radius >= edge_fade_radius && !is_dead()) {
			begin_edge_fade();
		}

		update_sprite_transform();
		update_sprite_color();
	}

	template <bool use_neon_maps, class M>
	void draw_as_sprite(
		augs::vertex_triangle& t1,
		augs::vertex_triangle& t2,
		const M& manager,
		const plain_animations_pool& anims
	) const {
		if (is_edge_fading()) {
			/*
				The size as of crossing the edge.
			*/

			auto frozen = sprite;
			frozen.current_lifetime_ms = edge_fade_start_ms;
			frozen.max_lifetime_ms = lifetime_before_edge_fade_ms;
			frozen.template draw_as_sprite<use_neon_maps>(t1, t2, manager, anims);
			return;
		}

		sprite.template draw_as_sprite<use_neon_maps>(t1, t2, manager, anims);
	}
};
