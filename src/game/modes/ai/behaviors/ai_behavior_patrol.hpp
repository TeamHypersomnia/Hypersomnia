#pragma once
#include "augs/math/vec2.h"
#include "augs/misc/randomization.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"
#include "game/enums/marker_type.h"

struct arena_mode_ai_team_state;
struct arena_mode_ai_state;
struct mode_player_id;

/*
	Patrol behavior.
	Bot is patrolling between waypoints in an assigned area.
	
	Patrol state includes:
	- Whether we're going to the first waypoint (can sprint/holster)
	- Current waypoint we're heading to
	- Camp state if the current waypoint is a camp waypoint
*/

struct ai_behavior_patrol {
	// GEN INTROSPECTOR struct ai_behavior_patrol
	entity_id current_waypoint;
	bool going_to_first_waypoint = true;
	bool walk_silently_to_next_waypoint = true;

	/*
		Camping state - only active if camp_timer > 0.
	*/
	float camp_timer = 0.0f;
	float camp_duration = 0.0f;
	vec2 camp_center = vec2::zero;
	vec2 camp_twitch_target = vec2::zero;
	vec2 camp_look_direction = vec2(1.0f, 0.0f);

	/*
		Twitch timers for camp movement.
	*/
	float twitch_move_timer = 0.0f;
	float twitch_still_timer = 0.0f;
	float twitch_move_duration = 0.0f;
	float twitch_still_duration = 0.0f;
	bool twitch_is_moving = false;

	/*
		Movement result from camp twitching (updated by process()).
	*/
	std::optional<vec2> twitch_direction;
	// END GEN INTROSPECTOR

	bool is_camping() const {
		return camp_timer > 0.0f && !going_to_first_waypoint;
	}

	bool operator==(const ai_behavior_patrol&) const = default;

	/*
		Update camp twitching behavior with breaks.
		
		Camp twitching works in phases:
		1) Twitch phase (100-300ms): Bot moves towards a random direction within 40px radius
		2) Still phase (500-800ms): Bot stays still
		
		If the bot exceeds the twitch radius boundary, it always moves back to center
		regardless of the current phase.
		
		At the start of each twitch phase, a new random target direction is chosen.
	*/
	void update_camp_twitch(
		const vec2 bot_pos,
		const real32 dt,
		randomization& rng
	) {
		constexpr float TWITCH_RADIUS = 40.0f;
		
		/* Twitch move duration: 100-300ms */
		constexpr float TWITCH_MOVE_MIN_SECS = 0.1f;
		constexpr float TWITCH_MOVE_MAX_SECS = 0.5f;
		
		/* Still (break) duration: 500-800ms */
		constexpr float TWITCH_STILL_MIN_SECS = 0.4f;
		constexpr float TWITCH_STILL_MAX_SECS = 3.0f;

		const auto offset_from_center = bot_pos - camp_center;
		const auto dist_from_center = offset_from_center.length();

		/*
			If we've exceeded the twitch radius, always go back to center.
			This takes priority over the twitch/still phase.
		*/
		if (dist_from_center > TWITCH_RADIUS) {
			camp_twitch_target = (camp_center - bot_pos).normalize();
		}

		/*
			Update twitch timers based on current phase.
		*/
		if (twitch_is_moving) {
			/* In twitch move phase. */
			twitch_move_timer -= dt;
			
			if (twitch_move_timer <= 0.0f) {
				/* Twitch move phase ended - start still phase. */
				twitch_is_moving = false;
				twitch_still_duration = rng.randval(TWITCH_STILL_MIN_SECS, TWITCH_STILL_MAX_SECS);
				twitch_still_timer = twitch_still_duration;
				twitch_direction = std::nullopt;
				return;
			}
			
			/* Continue moving towards twitch target. */
			const auto direction = camp_twitch_target;

			if (direction.is_nonzero()) {
				twitch_direction = vec2(direction).normalize();
			}
			else {
				twitch_direction = std::nullopt;
			}
		}
		else {
			/* In still (break) phase. */
			twitch_still_timer -= dt;
			
			if (twitch_still_timer <= 0.0f) {
				/* Still phase ended - start new twitch move phase. */
				twitch_is_moving = true;
				twitch_move_duration = rng.randval(TWITCH_MOVE_MIN_SECS, TWITCH_MOVE_MAX_SECS);
				twitch_move_timer = twitch_move_duration;
				
				/* Choose a new random twitch target direction. */
				const auto random_angle = rng.randval(0.0f, 360.0f);
				const auto random_offset = vec2::from_degrees(random_angle);
				camp_twitch_target = random_offset;

				if (dist_from_center > TWITCH_RADIUS) {
					camp_twitch_target = (camp_center - bot_pos).normalize();
				}
				
				/* Move towards the new target. camp_twitch_target is already a direction. */
				const auto direction = camp_twitch_target;
				if (direction.is_nonzero()) {
					twitch_direction = vec2(direction).normalize();
				}
				else {
					twitch_direction = std::nullopt;
				}
			}
			else {
				/* Still in break phase - don't move. */
				twitch_direction = std::nullopt;
			}
		}
	}

	/*
		Process patrol behavior for this frame.
		Handles:
		- Camp timer countdown
		- Camp twitch updates
		- Waypoint picking when path completes
	*/
	template <typename T>
	void process(
		T& cosm,
		arena_mode_ai_state& ai_state,
		arena_mode_ai_team_state& team_state,
		const mode_player_id& bot_player_id,
		const vec2 character_pos,
		const real32 dt_secs,
		randomization& rng,
		const bool pathfinding_just_completed
	);
};

