#pragma once
#include <cstddef>
#include <unordered_map>
#include <vector>
#include "augs/math/vec2.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/step_declaration.h"
#include "game/enums/faction_type.h"

/*
	View-side cache of enemy positions for the minimap.

	Enemies are shown at the last position where the viewed character
	or any conscious teammate had a line of sight to them, and briefly
	at the muzzle position whenever an audible enemy gunshot happens.

	Line of sight checks run once per logic step with a budget:
	the viewed character always checks, plus a single teammate
	chosen round-robin.
*/

class minimap_sighting_system {
public:
	struct enemy_record {
		vec2 last_seen_pos;

		/* All times below are in cosmos seconds (get_total_seconds_passed). */
		double last_seen_at = -1000.0;

		/* When the enemy (re)appeared - drives the pulse animation. */
		double appeared_at = -1000.0;

		double heard_at = -1000.0;
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
		double when = -1000.0;
	};

	struct event_pulse {
		vec2 pos;
		double when = -1000.0;

		/* When set, the pulse follows this entity instead of the fixed pos. */
		entity_id subject;
	};

	std::unordered_map<entity_id, enemy_record> enemy_records;
	std::vector<death_record> recent_deaths;

	/* Fired when the bomb lands on the ground or gets planted. */
	event_pulse bomb_pulse;

	void record_deaths(const const_logic_step step);

	void advance(
		const const_logic_step step,
		const entity_id viewed_character,
		const vec2 sight_range,
		const float fov_angle
	);

	bool is_seen_now(const enemy_record& rec, const double now_secs) const {
		const auto since_seen = now_secs - rec.last_seen_at;
		return since_seen >= 0.0 && since_seen <= visibility_grace_secs;
	}

	void reserve_caches_for_entities(const std::size_t) const {}
	void clear();

private:
	unsigned teammate_rotation_counter = 0;

	/* 0 - unknown, 1 - carried, 2 - dropped, 3 - planted */
	int prev_bomb_state = 0;
};
