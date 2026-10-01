#pragma once
#include "game/messages/damage_message.h"
#include "game/detail/physics/missile_surface_info.h"
#include "game/detail/sentience/sentience_getters.h"
#include "game/detail/missile/headshot_detection.hpp"
#include "game/detail/decals/spawn_decals.hpp"
#include "game/detail/decals/penetration_fatigue.h"
#include "game/detail/physics/calc_penetrability.hpp"
#include "game/stateless_systems/sentience_system.h"
#include "game/detail/missile/penetration_path.h"
#include "augs/misc/randomization.h"
#include "augs/templates/hash_templates.h"

#if HEADLESS
void draw_headshot_debug_lines(vec2, vec2, vec2, float) {}
#else
void draw_headshot_debug_lines(vec2 missile_pos, vec2 impact_dir, vec2 head_pos, float head_radius);
#endif

struct missile_collision_result {
	transformr transform_of_impact;
	vec2 impact_velocity;
	bool deleted_already = false;
	bool penetration_began = false;
};

enum class missile_collision_type {
	CONTACT_START,
	PRE_SOLVE,
};

/*
	Marks the surface a bullet has just entered and moves the impact effects onto the mark,
	so that the mark and the burst always match visually.
*/
template <class S>
void spawn_gunshot_decal_of_impact(
	const logic_step step,
	const components::missile& missile,
	const S& surface_handle,
	const b2Fixture* const fixture,
	const vec2 point,
	const vec2 impact_dir,
	messages::damage_message& damage_msg
) {
	const auto& cosm = step.get_cosmos();

	/*
		Seeded by the bullet's own seed, known since it was fired - see components::missile -
		and the surface and fixture, so that several entries of one bullet don't roll identical marks.
	*/
	auto rng = randomization(augs::hash_multiple(
		missile.rng_seed,
		surface_handle.get_id().raw.indirection_index,
		fixture != nullptr ? fixture->index_in_component : -1
	));

	/*
		Only evaluated if the decal has to stack in depth:
		how far this bullet would ACTUALLY get within the wall it has entered -
		the same walk the laser performs.
	*/
	auto get_max_decal_depth = [&]() {
		auto& obstacles = ::thread_local_penetration_obstacles();

		/* Only the wall it has entered matters. */
		::gather_obstacles_of_wall_run(cosm, point, impact_dir, obstacles);

		const auto walk = ::walk_penetration_obstacles(
			cosm,
			obstacles,
			point,
			impact_dir,
			missile.penetration_distance_remaining,
			missile.starting_penetration_distance,
			missile.penetration_fatigue_gift_used,
			missile.when_fired.step,
			[](auto&&...) {}
		);

		return std::min(
			walk.stopped_at.value_or(PENETRATION_PATH_MAX_RANGE_PX),
			::calc_wall_run_end(obstacles)
		);
	};

	const auto spawned_decal = ::spawn_surface_impact_decal(
		step,
		rng,
		surface_handle,
		fixture,
		point,
		impact_dir,
		damage_msg.damage.base,
		missile.decal_scale_of_sender,
		&material_decals_def::gunshot_decals,
		get_max_decal_depth
	);

	if (spawned_decal.has_value()) {
		damage_msg.point_of_impact = spawned_decal->pos;
	}
}

/*
	The damage of a bullet hitting something - less for every surface it went through before.
*/
template <class A>
messages::damage_message make_missile_damage_msg(
	const A& typed_missile,
	const invariants::missile& missile_def,
	const components::missile& missile
) {
	messages::damage_message damage_msg;
	damage_msg.damage = missile_def.damage;
	damage_msg.damage *= missile.power_multiplier_of_sender;
	damage_msg.origin = damage_origin(typed_missile);

	const auto dist_remaining = missile.penetration_distance_remaining;
	const auto dist_starting = missile.starting_penetration_distance;

	if (dist_remaining != dist_starting && dist_starting != 0.0f) {
		damage_msg.damage *= dist_remaining / dist_starting;
		damage_msg.origin.circumstances.wallbang = true;
	}

	return damage_msg;
}

/*
	While penetrating, a bullet gets no contact events with walls at all.
	advance_penetrations calls this when the bullet's path crosses into
	a surface that is not a part of the wall it was going through,
	so that the surface gets hit just as if the bullet had flown into it.
*/
template <class A, class S>
void on_missile_entered_next_surface(
	const logic_step step,
	const A& typed_missile,
	const S& surface_handle,
	const b2Fixture& fixture,
	const vec2 entry_point,
	const vec2 dir
) {
	const auto& missile_def = typed_missile.template get<invariants::missile>();
	const auto& missile = typed_missile.template get<components::missile>();

	auto damage_msg = ::make_missile_damage_msg(typed_missile, missile_def, missile);
	damage_msg.subject = surface_handle;
	damage_msg.impact_velocity = typed_missile.template get<components::rigid_body>().get_velocity();
	damage_msg.normal = -dir;
	damage_msg.point_of_impact = entry_point;
	damage_msg.spawn_destruction_effects = true;

	::spawn_gunshot_decal_of_impact(
		step,
		missile,
		surface_handle,
		std::addressof(fixture),
		entry_point,
		dir,
		damage_msg
	);

	step.post_message(damage_msg);
}

template <class A, class B>
static std::optional<missile_collision_result> collide_missile_against_surface(
	allocate_new_entity_access access,
	const logic_step step,

	const A& typed_missile,
	const B& surface_handle,

	const invariants::missile& missile_def,
	const components::missile& missile,

	const missile_collision_type type,
	const missile_surface_info& info,

	const b2Fixture_indices indices,

	const vec2& normal,
	const vec2& collider_impact_velocity,
	const vec2& point
) {
	const bool contact_start  = type == missile_collision_type::CONTACT_START;
	const bool pre_solve = type == missile_collision_type::PRE_SOLVE;

	auto& cosm = step.get_cosmos();
	const auto& clk = cosm.get_clock();
	const auto& now = clk.now;

	//LOG("(DET) MISSILE %x WITH: %x", pre_solve ? "PRE SOLVE" : "CONTACT START", surface_handle);
	//LOG_NVPS(point, typed_missile.get_logic_transform().pos);

	const auto& ricochet_cooldown_ms = missile_def.ricochet_cooldown_ms;
	(void)missile_def;
	(void)ricochet_cooldown_ms;

	const auto collision_normal = vec2(normal).normalize();

	if (info.should_ignore_altogether()) {
		RIC_LOG("IGNORED");
		return std::nullopt;
	}

	const bool should_send_damage =
		missile_def.damage_upon_collision
		&& !missile.deleted_already
	;

	if (!should_send_damage) {
		return std::nullopt;
	}

	if (pre_solve) {
		/* With a PreSolve must have happened a PostSolve that altered our initial velocity. */

		/* Not sure if this is needed here actually but let it be */
		make_velocity_face_body_orientation(typed_missile);
	}

	const bool within_ricochet_cooldown =
		missile.when_last_ricocheted.was_set()
		&& now.step <= missile.when_last_ricocheted.step + 1
	;

	if (within_ricochet_cooldown && pre_solve) {
		return std::nullopt;
	}

	if (within_ricochet_cooldown) {
		/*
			Only the fixture the bullet has just bounced off counts as that ricochet.
			Anything else it runs into meanwhile is where the ricocheted bullet ends up -
			e.g. the neighbouring wall of an inner corner.
		*/
		const auto* const fixture = ::find_fixture_of_impact(surface_handle, cosm.get_si(), point);
		const auto convex_index = fixture != nullptr ? static_cast<int32_t>(fixture->index_in_component) : -1;

		const bool same_fixture =
			surface_handle.get_id() == missile.last_ricochet_surface
			&& (missile.last_ricochet_convex_index == -1 || convex_index == -1 || convex_index == missile.last_ricochet_convex_index)
		;

		if (same_fixture) {
			RIC_LOG("DET: This impact counted as ricochet already.");
			return std::nullopt;
		}
	}

	/*
		Right after a ricochet, the contact's own velocity is the physics solver's bounce,
		not the ricochet - the bullet's current velocity is the one the ricochet gave it.
	*/
	const auto impact_velocity =
		within_ricochet_cooldown ?
		vec2(typed_missile.template get<components::rigid_body>().get_velocity()) :
		collider_impact_velocity
	;
	const auto impact_dir = vec2(impact_velocity).normalize();

	bool penetration_began = false;
	auto deleted_already = missile.deleted_already;

	const auto sentience_def = surface_handle.template find<invariants::sentience>();
	const auto sentience = surface_handle.template find<components::sentience>();

	const bool surface_sentient = sentience_def != nullptr && sentience != nullptr;

	const bool surface_is_wall = [&]() {
		if (const auto* const fixtures_def = surface_handle.template find<invariants::fixtures>()) {
			return ::is_penetrable_wall(fixtures_def->filter);
		}

		return false;
	}();

	if (contact_start && surface_is_wall) {
		/*
			Within the very step a bullet begins penetrating, the physics solver
			may already have bounced it off that wall into another one -
			such a contact is an artifact of the bounce. The walls truly
			along the bullet's path are hit by advance_penetrations.
		*/
		if (missile.during_penetration) {
			return std::nullopt;
		}

		/*
			A bullet moving away from a wall merely grazes it: e.g. right after
			a ricochet off the neighbouring tile of a flat wall, or while leaving
			the wall it has just gone through.
		*/
		if (impact_dir.dot(collision_normal) >= 0.f) {
			return std::nullopt;
		}
	}

	auto finalize_bullet = [&]() {
		if (!missile_def.destroy_upon_damage) {
			return;
		}

		deleted_already = true;

		step.queue_deletion_of(typed_missile, "Missile collision");

		auto rng = cosm.get_nontemporal_rng_for(typed_missile);

		if (!surface_sentient) {
			spawn_bullet_remnants(
				access,
				step,
				rng,
				missile_def.remnant_flavours,
				collision_normal,
				impact_dir,
				point
			);
		}
	};

	auto penetrate_or_finalize = [&]() {
		if (missile.penetration_distance_remaining > 0.0f) {
			if (!missile.during_penetration) {
				penetration_began = true;
			}
		}
		else {
			finalize_bullet();
		}
	};

	if (contact_start && !deleted_already) {
		auto damage_msg = ::make_missile_damage_msg(typed_missile, missile_def, missile);
		damage_msg.indices = indices;

		if (info.should_detonate()) {
			detonate_if(typed_missile.get_id(), point, step);

			{
				/* Startles as far as the full, undiminished damage would. */
				const auto total_damage_amount = missile_def.damage.base * missile.power_multiplier_of_sender;

				if (augs::is_positive_epsilon(total_damage_amount)) {
					startle_nearby_organisms(cosm, point, total_damage_amount * 12.f, 27.f, startle_type::LIGHTER);
					startle_nearby_organisms(cosm, point, total_damage_amount * 6.f, 50.f + total_damage_amount * 2.f, startle_type::IMMEDIATE);
				}
			}
		}

		damage_msg.subject = surface_handle;
		damage_msg.impact_velocity = impact_velocity;
		damage_msg.normal = collision_normal;
		damage_msg.point_of_impact = point;

		if (surface_sentient) {
			const auto missile_entity_id = typed_missile.get_id();
			const bool is_duplicate_bullet_hit =
				sentience->ignore_bullet == entity_id(missile_entity_id)
				&& sentience->ignore_bullet_when_born == missile.when_fired
			;

			if (is_duplicate_bullet_hit) {
				/*
					This bullet already damaged this sentience entity once.
					Ignore to prevent double damage from the same bullet.
				*/
				return std::nullopt;
			}

			const auto missile_pos = point;
			const auto head_transform = ::calc_head_transform(surface_handle);
			const auto head_radius = sentience_def->head_hitbox_radius * missile.head_radius_multiplier_of_sender;

			if (head_transform.has_value()) {
				const auto head_pos = head_transform->pos;

				::draw_headshot_debug_lines(missile_pos, impact_dir, head_pos, head_radius);

				if (::headshot_detected(
					missile_pos,
					impact_dir,
					head_pos,
					head_radius
				)) {
					damage_msg.origin.circumstances.headshot = true;
					damage_msg.headshot_mult = missile.headshot_multiplier_of_sender;
					damage_msg.head_transform = *head_transform;
				}
			}

			const bool was_conscious_before = sentience->is_conscious();

			sentience_system().process_damage_message(damage_msg, step);
			damage_msg.processed = true;

			const bool is_conscious_now = sentience->is_conscious();
			const bool just_died = was_conscious_before && !is_conscious_now;

			/*
				Store this bullet in ignore_bullet so that if the bullet
				continues to exist (e.g. character died), it won't damage again.
			*/
			sentience->ignore_bullet = entity_id(missile_entity_id);
			sentience->ignore_bullet_when_born = missile.when_fired;

			if (just_died) {
				/*
					When character receives damage that kills him, subtract as much health
					from the corpse as if it was hit by that bullet yet another time.
					This is done discreetly without showing additional damage indicators.
				*/
				auto& health = sentience->template get<health_meter_instance>();

				const auto additional_damage = damage_msg.damage.base; // * (damage_msg.origin.circumstances.headshot ? damage_msg.headshot_mult : 1.0f);

				/* A little less after all */
				if (true) {
					health.value -= additional_damage / 2.0f;
				}
			}

			if (sentience->is_conscious()) {
				finalize_bullet();
				damage_msg.spawn_destruction_effects = true;
			}
		}
		else {
			if (!info.ignore_standard_collision_resolution()) {
				penetrate_or_finalize();
				damage_msg.spawn_destruction_effects = true;

				/*
					A non-ricochet hit against a wall - ricochets returned early above.
					This also spawns on every penetration entry, as the bullet is not yet destroyed then.
				*/
				::spawn_gunshot_decal_of_impact(
					step,
					missile,
					surface_handle,
					::find_fixture_of_impact(surface_handle, cosm.get_si(), point),
					point,
					impact_dir,
					damage_msg
				);
			}
		}

		step.post_message(damage_msg);
	}

	return missile_collision_result { transformr { point, impact_dir.degrees() }, impact_velocity, deleted_already, penetration_began };
}
