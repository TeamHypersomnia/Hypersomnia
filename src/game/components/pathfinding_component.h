#pragma once
#include "augs/math/vec2.h"
#include "game/cosmos/entity_id.h"
#include "3rdparty/Box2D/Dynamics/b2Filter.h"
#include "augs/pad_bytes.h"

class pathfinding_system;

struct pathfinding_navigation_hint {
	// GEN INTROSPECTOR struct pathfinding_navigation_hint
	bool enabled = false;
	pad_bytes<3> pad;
	vec2 origin;
	vec2 target;
	// END GEN INTROSPECTOR
};

struct pathfinding_navigation_vertex {
	// GEN INTROSPECTOR struct pathfinding_navigation_vertex
	vec2 location;
	vec2 sensor;
	// END GEN INTROSPECTOR

	bool operator==(const pathfinding_navigation_vertex& b) const {
		return location == b.location && sensor == b.sensor;
	}
};

struct pathfinding_session {
	// GEN INTROSPECTOR struct pathfinding_session
	vec2 target;
	vec2 navigate_to;

	bool persistent_navpoint_set = false;
	pad_bytes<3> pad;
	pathfinding_navigation_vertex persistent_navpoint;

	std::vector<pathfinding_navigation_vertex> discovered_vertices;
	std::vector<pathfinding_navigation_vertex> undiscovered_vertices;
	std::vector<pathfinding_navigation_vertex> undiscovered_visible;

	float temporary_ignore_discontinuities_shorter_than = 0.f;
	// END GEN INTROSPECTOR

	bool operator==(const pathfinding_session&) const;
};

namespace components {
	struct pathfinding {
		static constexpr bool allow_nontriviality = true;

		static void clone_children(cosmos&, pathfinding&, const pathfinding&) { /* no children */ }

		// GEN INTROSPECTOR struct components::pathfinding
		float target_offset = 0.f;
		float rotate_navpoints = 0.f;
		float distance_navpoint_hit = 0.f;
		float starting_ignore_discontinuities_shorter_than = 100.f;

		b2Filter filter;
		bool force_persistent_navpoints = false;
		bool force_touch_sensors = false;

		bool enable_session_rollbacks = true;
		bool mark_touched_as_discovered = false;
		bool is_exploring = false;
		bool enable_backtracking = true;
		
		bool favor_velocity_parallellness = false;
		pad_bytes<3> pad;

		vec2 eye_offset;

		pathfinding_navigation_hint custom_exploration_hint;

		std::vector<pathfinding_session> session_stack;
		// END GEN INTROSPECTOR

		pathfinding_session& session();
		void start_pathfinding(vec2 target);
		void start_exploring();
		void restart_pathfinding();
		void stop_and_clear_pathfinding();

		const pathfinding_session& session() const;
		vec2 get_current_navigation_point() const;
		vec2 get_current_target() const;
		bool has_pathfinding_finished() const;
		bool has_exploring_finished() const;
		bool exists_through_undiscovered_visible(vec2 navpoint, float max_distance) const;

		void reset_persistent_navpoint();
	};
}