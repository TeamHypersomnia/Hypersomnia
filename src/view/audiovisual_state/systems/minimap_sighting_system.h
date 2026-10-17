#pragma once
#include <cstddef>
#include <optional>
#include <unordered_map>
#include <vector>
#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/step_declaration.h"
#include "game/enums/faction_type.h"
#include "game/modes/detail/fog_of_war_settings.h"

/*
	View-side cache of enemy positions for the minimap.

	Enemies are shown at the last position where the viewed character
	or any conscious teammate had them in view, and briefly
	at the muzzle position whenever an audible enemy gunshot happens.

	Line of sight checks run once per logic step with a budget:
	the viewed character always checks, plus a single teammate
	chosen round-robin.
*/

class minimap_sighting_system {
public:
	/* A time before anything could happen. */
	static constexpr double never_secs = -1000.0;

	struct enemy_record {
		vec2 last_seen_pos;

		/* All times below are in the seconds of clock_secs. */
		double last_seen_at = never_secs;

		/* When the enemy (re)appeared - drives the pulse animation. */
		double appeared_at = never_secs;

		double heard_at = never_secs;
		vec2 heard_pos;

		/*
			Whoever saw the enemy last keeps re-checking the line of sight
			every step, so the moment the sight is lost, the dot freezes
			exactly at the last truly seen position instead of jumping.
		*/
		entity_id last_seen_by;
	};

	static constexpr double visibility_grace_secs = 0.15;
	static constexpr double heard_shot_shows_for_secs = 1.0;
	static constexpr double pulse_duration_secs = 0.6;
	static constexpr double reappear_pulse_threshold_secs = 0.5;

	/*
		Deaths and knockouts of all characters, recorded from health events -
		robust against instant respawns and corpse deletion. Consumed by the
		tactical skull indicators.
	*/

	struct death_record {
		vec2 pos;
		faction_type faction = faction_type::SPECTATOR;
		double when = never_secs;
	};

	struct event_pulse {
		vec2 pos;
		double when = never_secs;

		/* When set, the pulse follows this entity instead of the fixed pos. */
		entity_id subject;
	};

	/*
		The real seconds of the last step recorded - all the records are timed by it.
		On a client, sightings are recorded from the referential cosmos only,
		while the viewed one is predicted ahead of it - so the records must never be compared with the viewed cosmos' clock.
	*/

	double clock_secs = 0.0;

	std::unordered_map<entity_id, enemy_record> enemy_records;
	std::vector<death_record> recent_deaths;

	/* Fired when the bomb lands on the ground or gets planted. */
	event_pulse bomb_pulse;

	/*
		What an observer sees. An enemy counts as seen only within a narrower cone
		of the view angle, so that enemies at the very edge of the view don't get pinned,
		and only on the screen - the camera follows the crosshair,
		so it is narrower than the whole fog of war.
	*/

	struct observer_view {
		fog_of_war_settings fow;
		ltrb screen_area;

		bool shows(const vec2 observer_pos, const vec2 look_dir, const vec2 enemy_pos) const;
	};

	/*
		A teammate's camera isn't known here, so they see what a player standing there would
		with the default camera settings and the crosshair not at the screen edge:
		the fog of war scaled by the camera zoom at their position,
		and a screen following their crosshair, showing the fog of war size (1920x1080 by default) -
		the camera zoom snaps to it - moved at most as far as a player's crosshair could move it.
	*/

	static observer_view calc_teammate_view(
		const fog_of_war_settings& rules_fow,
		const float camera_zoom,
		const vec2 teammate_pos,
		const vec2 camera_offset
	);

	void record_deaths(const const_logic_step step);

	void advance(
		const const_logic_step step,
		const entity_id viewed_character,
		const fog_of_war_settings& rules_fow,
		const std::optional<float> zoom_override,
		const fog_of_war_settings& viewed_fow,
		const ltrb viewed_screen_area
	);

	bool is_seen_now(const enemy_record& rec, const double now_secs) const {
		const auto since_seen = now_secs - rec.last_seen_at;
		return since_seen >= 0.0 && since_seen <= visibility_grace_secs;
	}

	void reserve_caches_for_entities(const std::size_t) const {}
	void clear();

private:
	enum class bomb_state_type {
		UNKNOWN,
		CARRIED,
		DROPPED,
		PLANTED
	};

	unsigned teammate_rotation_counter = 0;
	bomb_state_type prev_bomb_state = bomb_state_type::UNKNOWN;
};
