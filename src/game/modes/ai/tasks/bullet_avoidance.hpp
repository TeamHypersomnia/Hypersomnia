#pragma once
#include "augs/misc/randomization.h"
#include "augs/misc/randomization_declaration.h"
#include "augs/math/repro_math.h"
#include "game/components/fixtures_component.h"
#include "game/components/missile_component.h"
#include "game/components/movement_component.h"
#include "game/components/sender_component.h"
#include "game/cosmos/for_each_entity.h"
#include "game/enums/filters.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/ai/tasks/line_of_sight.hpp"

/*
	Bullet avoidance — highest-priority emergency movement layer.

	Scans for enemy missiles on a collision course with the bot.
	Applies a perpendicular strafe (CW or CCW relative to the bullet's
	trajectory) for a randomly chosen duration after detecting the threat.

	Collision course detection: project the bot's center onto the bullet's
	trajectory line and raycast from the bullet's position towards that
	projection using the missile's actual physics filter.  If the only
	intersection is the bot's body, the bullet is on course.

	CW/CCW decision: sense_free_space_from_vertices_of raycasts from all
	polygon corners of the bot in each perpendicular direction; the side with
	more total free space wins.

	The dodge direction is cached and applied for BULLET_AVOIDANCE_EVASION_TIMER
	(200–300 ms, randomised by cosmos total steps) regardless of whether the
	bullet is still alive.
*/

/* Search radius: missiles farther than this are ignored. */
constexpr float BULLET_AVOIDANCE_QUERY_HALF_SIDE = 4500.0f;

/* Bullets slower than this (px/s squared) are ignored (e.g. rolling grenades). */
constexpr float BULLET_AVOIDANCE_MIN_SPEED_SQ = 200.0f * 200.0f;

/* Length of per-vertex raycasts used to pick CW vs CCW dodge direction (pixels). */
constexpr float BULLET_AVOIDANCE_RAYCAST_LENGTH = 130.0f;

/*
	When the free-space difference between CW and CCW is smaller than this
	threshold (pixels), fall back to the impact-point tiebreaker instead of
	blindly picking the marginally larger side.
*/
constexpr float BULLET_AVOIDANCE_SPACE_TIE_THRESHOLD = 20.0f;

/* Evasion timer range (milliseconds). Seeded by cosmos total steps. */
constexpr float BULLET_AVOIDANCE_EVASION_TIMER_MIN_MS = 200.0f;
constexpr float BULLET_AVOIDANCE_EVASION_TIMER_MAX_MS = 300.0f;

inline bool update_bullet_avoidance(
	const ai_character_context& ctx,
	components::movement& movement,
	const float dt_secs,
	const difficulty_type difficulty,
	const bool should_run_avoidance_update,
	const bool is_freeze_time,
	const bool is_thinking_what_to_buy
) {
	if (is_freeze_time) {
		return false;
	}

	auto& ai_state = ctx.ai_state;
	const auto& cosm = ctx.cosm;
	const auto& character_handle = ctx.character_handle;
	const auto character_pos = ctx.character_pos;
	const auto& physics = ctx.physics;
	const auto si = cosm.get_si();
	const auto& clk = cosm.get_clock();

	if (should_run_avoidance_update) {
		const auto bot_faction = character_handle.get_official_faction();
		const auto character_unversioned_id = character_handle.get_id().to_unversioned();
		const auto min_bullet_age_secs = ::get_reaction_time_secs(difficulty) / 1.4f;

		struct bullet_candidate {
			signi_entity_id id;
			float dist_sq;
			vec2 intersection; /* where the bullet would first hit the character */
		};

		std::optional<bullet_candidate> closest;

		cosm.for_each_having<components::missile>([&](const auto& bullet_handle) {
			/* Skip friendly missiles */
			if (const auto* sender = bullet_handle.template find<components::sender>()) {
				if (sender->faction_of_sender == bot_faction) {
					return;
				}
			}

			const auto bullet_pos = bullet_handle.get_logic_transform().pos;
			const auto delta = character_pos - bullet_pos;
			const auto dist_sq = delta.length_sq();

			if (dist_sq > BULLET_AVOIDANCE_QUERY_HALF_SIDE * BULLET_AVOIDANCE_QUERY_HALF_SIDE) {
				return;
			}

			const auto bullet_vel = bullet_handle.get_effective_velocity();

			if (bullet_vel.length_sq() < BULLET_AVOIDANCE_MIN_SPEED_SQ) {
				return;
			}

			/* Respect bot reaction time: ignore bullets that are too fresh */
			{
				const auto bullet_age_secs = (clk.now - bullet_handle.when_born()).in_seconds(clk.dt);

				if (bullet_age_secs < min_bullet_age_secs) {
					return;
				}
			}

			const auto bullet_dir = vec2(bullet_vel).normalize();

			/*
				Project the bot's position onto the bullet's trajectory.
				A negative projection means the bullet is heading away.
			*/
			const auto proj_dist = delta.dot(bullet_dir);

			if (proj_dist <= 0.0f) {
				return;
			}

			const auto projected_pos = bullet_pos + bullet_dir * proj_dist;

			/* Use the missile's actual collision filter for the raycast */
			const auto* fix_inv = bullet_handle.template find<invariants::fixtures>();

			if (fix_inv == nullptr) {
				return;
			}

			/*
				Raycast from the bullet towards the projected point.
				Collision course confirmed only if the bot's body is the first hit.
			*/
			const auto raycast = physics.ray_cast_px(
				si,
				bullet_pos,
				projected_pos,
				fix_inv->filter,
				bullet_handle.get_id()
			);

			if (!raycast.hit || raycast.what_entity != character_unversioned_id) {
				return;
			}

			if (!closest.has_value() || dist_sq < closest->dist_sq) {
				closest = bullet_candidate{ bullet_handle.get_id(), dist_sq, raycast.intersection };
			}
		});

		if (closest.has_value()) {
			if (ai_state.avoided_bullet != closest->id) {
				/* New bullet — decide CW or CCW once using per-vertex free-space sensing */
				ai_state.avoided_bullet = closest->id;

				const auto threat = cosm[closest->id];
				const auto bullet_vel = threat.get_effective_velocity();
				const auto bullet_dir = vec2(bullet_vel).normalize();

				const auto perp_cw  = bullet_dir.perpendicular_cw();
				const auto perp_ccw = bullet_dir.perpendicular_ccw();

				const auto filter = predefined_queries::pathfinding();

				const auto space_cw = ::sense_free_space_from_vertices_of(
					character_handle,
					perp_cw,
					BULLET_AVOIDANCE_RAYCAST_LENGTH,
					physics,
					si,
					filter
				);

				const auto space_ccw = ::sense_free_space_from_vertices_of(
					character_handle,
					perp_ccw,
					BULLET_AVOIDANCE_RAYCAST_LENGTH,
					physics,
					si,
					filter
				);

				if (repro::fabs(space_cw - space_ccw) < BULLET_AVOIDANCE_SPACE_TIE_THRESHOLD) {
					/*
						Sides are roughly equal — dodge away from the half of the
						body the bullet is actually aimed at.
						If the impact is on the CW side, strafe CCW, and vice-versa.
					*/
					const auto to_impact = closest->intersection - character_pos;
					ai_state.avoided_bullet_dir = (to_impact.dot(perp_cw) >= 0.0f) ? -1 : 1;
				}
				else {
					ai_state.avoided_bullet_dir = (space_cw >= space_ccw) ? 1 : -1;
				}

				ai_state.avoided_bullet_dodge = (ai_state.avoided_bullet_dir > 0)
					? perp_cw
					: perp_ccw
				;

				/*
					Set evasion timer: random 200–300 ms, seeded by cosmos total steps
					so the duration varies between encounters but is deterministic.
				*/
				auto rng = randomization(static_cast<rng_seed_type>(cosm.get_total_steps_passed()));
				ai_state.avoided_bullet_timer = rng.randval(
					BULLET_AVOIDANCE_EVASION_TIMER_MIN_MS,
					BULLET_AVOIDANCE_EVASION_TIMER_MAX_MS
				) / 1000.0f;
			}
			/* else: same bullet — keep cached direction and let timer run */
		}
		else {
			ai_state.avoided_bullet = {};
		}
	}

	/* Tick the evasion timer and apply the cached dodge direction every frame */
	if (ai_state.avoided_bullet_timer < 0.0f) {
		return false;
	}

	ai_state.avoided_bullet_timer -= dt_secs;

	if (ai_state.avoided_bullet_timer < 0.0f) {
		ai_state.avoided_bullet_timer = -1.0f;
		ai_state.avoided_bullet = {};
		return false;
	}

	if (is_thinking_what_to_buy) {
		return false;
	}

	movement.flags.set_from_closest_direction(ai_state.avoided_bullet_dodge);

	return true;
}

