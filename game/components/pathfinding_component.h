#pragma once
#include "augs/math/vec2.h"
#include "game/transcendental/entity_id.h"
#include "3rdparty/Box2D/Dynamics/b2Fixture.h"
#include "augs/padding_byte.h"

class pathfinding_system;

namespace components {
	struct pathfinding  {
		typedef std::pair<vec2, vec2> edge;

		struct navigation_hint {
			bool enabled = false;
			vec2 origin, target;
		};

		struct pathfinding_session {
			struct navigation_vertex {
				vec2 location;
				vec2 sensor;
			};

			vec2 target;
			vec2 navigate_to;

			bool persistent_navpoint_set = false;
			navigation_vertex persistent_navpoint;

			std::vector<navigation_vertex> discovered_vertices;
			std::vector<navigation_vertex> undiscovered_vertices;
			std::vector<navigation_vertex> undiscovered_visible;

			float temporary_ignore_discontinuities_shorter_than = 0.f;
		};


		float target_offset = 0.f;
		float rotate_navpoints = 0.f;
		float distance_navpoint_hit = 0.f;
		float starting_ignore_discontinuities_shorter_than = 100.f;

		bool force_persistent_navpoints = false;
		bool force_touch_sensors = false;
		bool enable_session_rollbacks = true;
		bool mark_touched_as_discovered = false;

		vec2 eye_offset;

		navigation_hint custom_exploration_hint;

		bool is_exploring = false;
		std::vector <pathfinding_session> session_stack;

		b2Filter filter;

		bool enable_backtracking = true;

		/* only in the context of exploration
		will pick vertices that are the most parallell with the velocity
		*/
		bool favor_velocity_parallellness = false;

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