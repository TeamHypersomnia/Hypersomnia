#pragma once
#include <Box2D/Collision/Shapes/b2PolygonShape.h>
#include <Box2D/Collision/b2Collision.h>
#include "augs/math/repro_math.h"
#include "game/components/movement_component.h"
#include "game/components/sentience_component.h"
#include "game/cosmos/for_each_entity.h"
#include "game/detail/sentience/sentience_getters.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"
#include "game/modes/ai/tasks/line_of_sight.hpp"

/*
	Emergency avoidance layer — bot-to-bot collision avoidance component.

	Called from the emergency avoidance layer (PHASE 8) alongside
	update_danger_avoidance. should_run_avoidance_update must be computed
	by the caller and shared between both avoidance functions.

	The result is cached in ai_state.avoidance_dir and applied every frame
	until the next recompute for this bot.
*/

inline bool update_bot_avoidance(
	const ai_character_context& ctx,
	components::movement& movement,
	const bool should_run_avoidance_update,
	const bool is_freeze_time,
	const bool is_thinking_what_to_buy
) {
	constexpr float AVOIDANCE_QUERY_HALF_SIDE = 250.0f;
	constexpr float AVOIDANCE_LOOKAHEAD_SECS = 0.2f;
	constexpr float AVOIDANCE_CHARACTER_HALF_SIZE = 14.0f;
	constexpr float AVOIDANCE_MIN_VELOCITY_SQ = 1.0f;

	auto& ai_state = ctx.ai_state;
	const auto& cosm = ctx.cosm;
	const auto& character_handle = ctx.character_handle;
	const auto character_pos = ctx.character_pos;
	const auto& physics = ctx.physics;

	if (should_run_avoidance_update) {
		ai_state.avoidance_dir = std::nullopt;

		const auto my_vel = character_handle.get_logic_transform().get_direction() * 450;

		if (my_vel.length_sq() > AVOIDANCE_MIN_VELOCITY_SQ) {
			const auto si = cosm.get_si();
			const auto my_vel_norm = vec2(my_vel).normalize();

			auto cancel_camping = [&]() {
				if (auto* patrol = ::get_behavior_if<ai_behavior_patrol>(ai_state.last_behavior)) {
					if (patrol->is_camping()) {
						/*
							Zero the timer but leave patrol_waypoint/push_waypoint and
							camp_duration intact. The patrol process will trigger its
							"timer expired" branch next tick with current_waypoint_id
							still set to the old camp spot, so
							find_random_unassigned_patrol_waypoint receives it as the
							ignore parameter and will never re-pick it.

							Works for push waypoint camping too: is_camping() only
							checks camp_timer, and clear_waypoint() clears both push
							and patrol waypoints.
						*/
						patrol->camp_timer = 0.0f;
						patrol->twitch_direction = std::nullopt;
					}
				}
			};

			cosm.for_each_having<components::sentience>(
				[&](const auto& other) {
					if (ai_state.avoidance_dir.has_value()) {
						return;
					}

					if (other == character_handle || !other.alive() || !::sentient_and_conscious(other)) {
						return;
					}

					const auto other_pos = other.get_logic_transform().pos;
					const auto delta = other_pos - character_pos;

					if (repro::fabs(delta.x) > AVOIDANCE_QUERY_HALF_SIDE ||
						repro::fabs(delta.y) > AVOIDANCE_QUERY_HALF_SIDE) {
						return;
					}

					if (!::is_in_line_of_sight(character_pos, other_pos, physics, cosm, character_handle)) {
						return;
					}

					const auto other_vel = other.get_logic_transform().get_direction() * 450.0f;

					/* Build swept AABB for self: center shifted by vel*dt/2, half-size expanded by |vel|*dt/2 */
					const auto my_center = character_pos + my_vel * (AVOIDANCE_LOOKAHEAD_SECS * 0.5f);
					const auto my_half_x = AVOIDANCE_CHARACTER_HALF_SIZE + repro::fabs(my_vel.x) * (AVOIDANCE_LOOKAHEAD_SECS * 0.5f);
					const auto my_half_y = AVOIDANCE_CHARACTER_HALF_SIZE + repro::fabs(my_vel.y) * (AVOIDANCE_LOOKAHEAD_SECS * 0.5f);

					/* Build swept AABB for other */
					const auto other_center = other_pos + other_vel * (AVOIDANCE_LOOKAHEAD_SECS * 0.5f);
					const auto other_half_x = AVOIDANCE_CHARACTER_HALF_SIZE + repro::fabs(other_vel.x) * (AVOIDANCE_LOOKAHEAD_SECS * 0.5f);
					const auto other_half_y = AVOIDANCE_CHARACTER_HALF_SIZE + repro::fabs(other_vel.y) * (AVOIDANCE_LOOKAHEAD_SECS * 0.5f);

					b2PolygonShape shape_a;
					shape_a.SetAsBox(si.get_meters(my_half_x), si.get_meters(my_half_y));

					b2PolygonShape shape_b;
					shape_b.SetAsBox(si.get_meters(other_half_x), si.get_meters(other_half_y));

					const auto xf_a = transformr(my_center, 0.0f).to<b2Transform>(si);
					const auto xf_b = transformr(other_center, 0.0f).to<b2Transform>(si);

					if (!b2TestOverlap(&shape_a, 0, &shape_b, 0, xf_a, xf_b)) {
						return;
					}

					/*
						Collision course confirmed — always cancel camping regardless of
						relative velocity direction or whether the other character is a bot.
					*/
					cancel_camping();

					/* Only steer away from other bots, not human players */
					if (const auto* s = other.template find<components::sentience>()) {
						if (!s->is_bot) {
							return;
						}
					}

					/*
						Only steer if the other bot is coming from the opposite half-circle
						(dot < 0) or is stationary.  Bots running in the same general
						direction (dot >= 0) don't need active steering — they'll sort
						themselves out through physics.
					*/
					if (other_vel.length_sq() < AVOIDANCE_MIN_VELOCITY_SQ) {
						/* Stationary obstacle in our path — steer CW */
						ai_state.avoidance_dir = my_vel_norm.perpendicular_cw();
					}
					else {
						const auto other_vel_norm = vec2(other_vel).normalize();

						if (my_vel_norm.dot(other_vel_norm) < 0.0f) {
							/* Approaching from opposite direction — steer CW */
							ai_state.avoidance_dir = my_vel_norm.perpendicular_cw();
						}
					}
				}
			);
		}
	}

	/* Apply cached avoidance direction, overriding pathfinding movement */
	const auto* const defuse_behavior = ::get_behavior_if<ai_behavior_defuse>(ai_state.last_behavior);
	const bool is_actively_defusing = defuse_behavior != nullptr && defuse_behavior->is_defusing;

	if (ai_state.avoidance_dir.has_value() && !is_thinking_what_to_buy && !is_freeze_time && !is_actively_defusing) {
		movement.flags.set_from_closest_direction(*ai_state.avoidance_dir);
		return true;
	}

	return false;
}
