#pragma once

#include "math/vec2.h"

#include "visibility_component.h"

namespace components {
	struct pathfinding  {
		typedef std::pair<vec2, vec2> edge;

		struct navigation_hint {
			bool enabled = false;
			vec2 origin, target;
		} custom_exploration_hint;

		struct pathfinding_session {
			vec2 target, navigate_to;

			struct navigation_vertex {
				vec2 location, sensor;
			};

			bool persistent_navpoint_set = false;
			navigation_vertex persistent_navpoint;

			std::vector<navigation_vertex> discovered_vertices, undiscovered_vertices, undiscovered_visible;
			float temporary_ignore_discontinuities_shorter_than = 0.f;
		};

		bool enable_backtracking = true;
		bool is_exploring = false;

		/* only in the context of exploration 
		will pick vertices that are the most parallell with the velocity
		*/
		bool favor_velocity_parallellness = false;

		float target_offset = 0.f;
		float rotate_navpoints = 0.f;
		float distance_navpoint_hit = 0.f;
		float starting_ignore_discontinuities_shorter_than = 100.f;

		bool force_persistent_navpoints = false;
		bool force_touch_sensors = false;
		bool enable_session_rollbacks = true;
		bool mark_touched_as_discovered = false;

		vec2 eye_offset;

		std::function<bool(augs::entity_id, vec2, vec2)> first_priority_navpoint_check;
		std::function<bool(augs::entity_id, vec2, vec2)> target_visibility_condition;

		std::vector <pathfinding_session> session_stack;

		pathfinding_session& session() {
			return session_stack.back();
		}

		void start_pathfinding(vec2 target) {
			clear_pathfinding_info();
			session_stack.push_back(pathfinding_session());
			session().target = target;
			session().temporary_ignore_discontinuities_shorter_than = starting_ignore_discontinuities_shorter_than;
			is_exploring = false;
		}

		void start_exploring() {
			clear_pathfinding_info();
			session_stack.push_back(pathfinding_session());
			is_exploring = true;
		}

		void reset_persistent_navpoint() {
			session().persistent_navpoint_set = false;
		}

		void clear_pathfinding_info() {
			session_stack.clear();
			is_exploring = false;
		}

		void clear_internal_data() {
			session_stack.resize(1);
			session().discovered_vertices.clear();
			session().undiscovered_visible.clear();
			session().undiscovered_vertices.clear();
			session().persistent_navpoint_set = false;
		}

		const pathfinding_session& session() const {
			return session_stack.back();
		}

		vec2 get_current_navigation_target() const {
			return session().navigate_to;
		}

		vec2 get_current_target() const {
			return session().target;
		}

		bool is_still_pathfinding() const {
			return !session_stack.empty();
		}

		bool is_still_exploring() const {
			return is_exploring;
		}

		bool exists_through_undiscovered_visible(vec2 navpoint, float max_distance) const {
			for (auto& memorised_undiscovered_visible : session().undiscovered_visible) {
				/* if a discontinuity with the same closer vertex already exists */
				if ((memorised_undiscovered_visible.sensor - navpoint).length_sq() < max_distance * max_distance) {
					return true;
				}
			}

			return false;
		}
	};
}