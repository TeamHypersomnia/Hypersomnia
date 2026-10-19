#include <cmath>
#include <algorithm>
#include <limits>
#include "augs/misc/randomization.h"
#include "augs/templates/container_templates.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"
#include "game/cosmos/entity_handle.h"
#include "view/viewables/particle_effect.h"
#include "view/view_container_sizes.h"
#include "view/audiovisual_state/systems/portal_particles_spawner.h"
#include "view/audiovisual_state/systems/particles_simulation_system.h"
#include "view/audiovisual_state/special_effects_settings.h"

/*
	Ages a portal particle just spawned by age_secs at once, in closed form -
	its motion is simple enough: a constant radial speed (stopping at max_radius if stop_at_max_radius)
	and a constant spin, as explosion_particle::integrate would do it with the infinitesimal steps.
	Returns false if it would have died by then.
*/

static bool age_portal_particle(portal_particle& p, const float age_secs) {
	const auto age_ms = age_secs * 1000.f;
	const auto start_radius = p.radius;

	auto radius_at = [&](const float t_secs) {
		const auto r = start_radius + p.radial_vel * t_secs;
		return p.stop_at_max_radius ? std::min(r, p.max_radius) : r;
	};

	const auto end_radius = radius_at(age_secs);

	if (end_radius < 0.f || (!p.stop_at_max_radius && end_radius > p.max_radius)) {
		return false;
	}

	if (p.edge_fade_ms > 0.f && start_radius < p.edge_fade_radius && end_radius >= p.edge_fade_radius && p.radial_vel > 0.f) {
		const auto crossing_ms = 1000.f * (p.edge_fade_radius - start_radius) / p.radial_vel;

		if (crossing_ms >= p.sprite.max_lifetime_ms) {
			return false;
		}

		p.sprite.current_lifetime_ms = crossing_ms;
		p.begin_edge_fade();
	}

	if (age_ms >= p.sprite.max_lifetime_ms) {
		return false;
	}

	p.sprite.current_lifetime_ms = age_ms;
	p.radius = end_radius;
	p.angle += p.angular_vel * age_secs;

	if (p.stop_at_max_radius && end_radius >= p.max_radius) {
		p.radial_vel = 0.f;
	}

	p.update_sprite_transform();
	p.update_sprite_color();

	return true;
}

/*
	See continuous_rings_particles.

	A portal counts as in view when its circle touches the queried camera
	(already a bit bigger than the screen, so that it is filled up right before it shows up),
	its particles dropped as soon as it is out of it.
	A portal coming into view spawns all its particles at once, already aged as if they had been there all along.

	All the portals in view share PORTAL_PARTICLES_BUDGET, each scaled down equally once they would exceed it.
	A portal over its share (e.g. once another one comes into view) just stops spawning until its excess dies out.
*/

void portal_particles_spawner::advance(
	const cosmos& cosm,
	randomization& rng,
	const ltrb queried_camera_aabb,
	const explosions_settings& settings,
	const particles_emission& emission,
	particles_simulation_system& particles,
	const float amount_mult,
	const float dt_secs
) {
	/*
		The particle count is tuned for a portal of this radius - smaller ones get proportionally fewer
		(with scale_count_with_size).
	*/

	constexpr auto reference_portal_radius = 384.f;

	visible_portals.clear();
	visible_portal_indices.clear();

	cosm.for_each_having<components::portal>(
		[&](const auto& typed_portal_handle) {
			const auto& portal = typed_portal_handle.template get<components::portal>();

			if (!portal.rings_effect.is_enabled || !portal.rings_effect.value.pixel_particles || amount_mult <= 0.f) {
				return;
			}

			const auto radius = typed_portal_handle.get_logical_size().smaller_side() / 2;
			const auto center = typed_portal_handle.get_logic_transform().pos;

			if (!queried_camera_aabb.hover(ltrb::center_and_size(center, vec2::square(radius * 2)))) {
				return;
			}

			const auto& e = portal.rings_effect.value;
			const auto area_mult = 
				e.particles.scale_count_with_size ?
				std::min(1.f, (radius / reference_portal_radius) * (radius / reference_portal_radius)) :
				1.f
			;

			auto v = visible_portal();
			v.id = typed_portal_handle.get_id();
			v.center = center;
			v.radius = radius;
			v.rings = std::addressof(e);
			v.count = static_cast<float>(std::max(0, e.particles.count)) * area_mult * std::max(0.f, amount_mult);

			visible_portal_indices[v.id] = visible_portals.size();
			visible_portals.push_back(v);
		}
	);

	/*
		Drop the particles of the portals out of view, count the others'.
		A portal's particles are spawned together, so they mostly come in runs - one lookup per run.
	*/

	{
		auto last_owner = entity_id();
		visible_portal* last_portal = nullptr;

		for (auto& p : particles.portal_particles) {
			if (p.owner != last_owner) {
				last_owner = p.owner;

				const auto found = visible_portal_indices.find(p.owner);
				last_portal = found != visible_portal_indices.end() ? std::addressof(visible_portals[found->second]) : nullptr;
			}

			if (last_portal == nullptr) {
				p.sprite.current_lifetime_ms = p.sprite.max_lifetime_ms;
			}
			else if (!p.is_dead()) {
				++last_portal->alive;
			}
		}
	}

	const auto& definitions = emission.get_definitions<general_particle>();

	auto total_count = 0.f;

	for (const auto& v : visible_portals) {
		total_count += v.count;
	}

	const auto budget_mult = 
		total_count > static_cast<float>(PORTAL_PARTICLES_BUDGET) ?
		static_cast<float>(PORTAL_PARTICLES_BUDGET) / total_count :
		1.f
	;

	auto opaque = [](rgba c) {
		c.a = 255;
		return c;
	};

	for (const auto& v : visible_portals) {
		if (definitions.empty()) {
			break;
		}

		const auto& e = *v.rings;
		const auto& def = e.particles;

		const auto lifetime_secs = std::max(0.1f, def.lifetime_secs);
		const auto lifetime_ms = lifetime_secs * 1000.f;

		const auto target = v.count * budget_mult;
		const bool just_came_into_view = !found_in(portals_visible_last_frame, v.id);

		const auto expected_amount = [&]() {
			if (just_came_into_view) {
				return std::max(0.f, target - static_cast<float>(v.alive));
			}

			if (static_cast<float>(v.alive) >= target) {
				return 0.f;
			}

			return target * dt_secs / lifetime_secs;
		}();

		const auto max_age_secs = just_came_into_view ? lifetime_secs : dt_secs;

		const auto whole_amount = static_cast<int>(expected_amount);
		const auto amount = whole_amount + (rng.randval(0.f, 1.f) < expected_amount - static_cast<float>(whole_amount) ? 1 : 0);

		const auto radius = v.radius;
		const bool has_edge = def.edge_radius_mult > 0.f;
		const auto edge_radius = has_edge ? radius * def.edge_radius_mult : std::numeric_limits<float>::max();
		const auto spawn_radius = std::min(radius, edge_radius);

		const auto size_mult = settings.fire_particles.particle_size_mult * def.size_mult;

		for (int i = 0; i < amount; ++i) {
			auto p = portal_particle();
			p.owner = v.id;

			p.sprite = definitions[rng.randval(0u, static_cast<unsigned>(definitions.size()) - 1)];
			p.sprite.multiply_size(size_mult);
			p.sprite.max_lifetime_ms = lifetime_ms;
			p.sprite.unshrinking_time_ms = lifetime_ms * std::max(0.f, def.grow_in);
			p.sprite.shrink_when_ms_remaining = lifetime_ms * std::max(0.f, def.shrink_out);

			/* Uniform density over the disc. */
			p.center = v.center;
			p.radius = spawn_radius * std::sqrt(rng.randval(0.f, 1.f));
			p.angle = rng.randval(0.f, 360.f);
			p.radial_vel = radius * rng.randval(def.radial_speed_min, def.radial_speed_max);
			p.angular_vel = def.spin * rng.randval(0.5f, 1.5f);

			const bool leaks = has_edge && rng.randval(0.f, 1.f) < def.edge_leak;

			if (leaks) {
				p.edge_fade_radius = edge_radius;
				p.edge_fade_ms = std::max(0.01f, def.leak_fade_secs) * 1000.f;
			}
			else if (has_edge) {
				p.max_radius = edge_radius * (1.f - std::clamp(def.edge_softness, 0.f, 1.f) * rng.randval(0.f, 1.f));
				p.stop_at_max_radius = true;
			}

			const auto start_color = opaque(rng.randval(0, 1) == 0 ? e.inner_color : e.outer_color);

			p.hot_color = start_color;
			p.ring_color = start_color;
			p.cool_color = opaque(e.inner_color);
			p.cool_from_ms = lifetime_ms * def.end_color_from;
			p.cool_until_ms = p.cool_from_ms + lifetime_ms * 0.1f;

			p.fade_in_ms = lifetime_ms * std::max(0.f, def.fade_in);
			p.fade_out_ms = lifetime_ms * std::max(0.f, def.fade_out);
			p.max_alpha = std::clamp(def.max_opacity, 0.f, 1.f);

			if (::age_portal_particle(p, rng.randval(0.f, max_age_secs))) {
				particles.add_portal_particle(p);
			}
		}
	}

	portals_visible_last_frame.clear();

	for (const auto& v : visible_portals) {
		portals_visible_last_frame.insert(v.id);
	}
}

void portal_particles_spawner::clear() {
	visible_portals.clear();
	visible_portal_indices.clear();
	portals_visible_last_frame.clear();
}
