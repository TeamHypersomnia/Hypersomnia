#pragma once
#include "entity_system/component.h"
#include "math/vec2.h"

#include "visibility_component.h"

namespace components {
	struct pathfinding : public augs::entity_system::component {
		typedef std::pair<augs::vec2<>, augs::vec2<>> edge;

		pathfinding() : force_touch_sensors(false), enable_session_rollbacks(true), mark_touched_as_discovered(false), force_persistent_navpoints(false), favor_velocity_parallellness(false), enable_backtracking(true), target_offset(0.f), distance_navpoint_hit(0.f), is_exploring(false), rotate_navpoints(0.f) {}

		bool enable_backtracking;
		bool is_exploring;

		/* only in the context of exploration 
		will pick vertices that are the most parallell with the velocity
		*/
		bool favor_velocity_parallellness;

		float target_offset;
		float rotate_navpoints;
		float distance_navpoint_hit;
		float starting_ignore_discontinuities_shorter_than;

		struct navigation_hint {
			bool enabled;
			augs::vec2<> origin, target;
			navigation_hint() : enabled(false) {}
		} custom_exploration_hint;

		bool force_persistent_navpoints;
		bool force_touch_sensors;
		bool enable_session_rollbacks;
		bool mark_touched_as_discovered;

		std::function<bool(augs::entity_system::entity_id, augs::vec2<>, augs::vec2<>)> 
			first_priority_navpoint_check, target_visibility_condition;

		augs::vec2<> eye_offset;

		struct pathfinding_session {
			augs::vec2<> target, navigate_to;
			
			struct navigation_vertex {
				augs::vec2<> location, sensor;
			};

			bool persistent_navpoint_set;
			navigation_vertex persistent_navpoint;

			std::vector<navigation_vertex> discovered_vertices, undiscovered_vertices, undiscovered_visible;
			float temporary_ignore_discontinuities_shorter_than;

			pathfinding_session() : temporary_ignore_discontinuities_shorter_than(0.f), persistent_navpoint_set(false) {}
		};

		std::vector <pathfinding_session> session_stack;

		pathfinding_session& session() {
			return session_stack.back();
		}

		void start_pathfinding(augs::vec2<> target) {
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

		augs::vec2<> get_current_navigation_target() {
			return session().navigate_to;
		}

		augs::vec2<> get_current_target() {
			return session().target;
		}

		void reset_persistent_navpoint() {
			session().persistent_navpoint_set = false;
		}

		bool is_still_pathfinding() const {
			return !session_stack.empty();
		}

		bool is_still_exploring() const {
			return is_exploring;
		}

		bool exists_through_undiscovered_visible(augs::vec2<> navpoint, float max_distance) {
			for (auto& memorised_undiscovered_visible : session().undiscovered_visible) {
				/* if a discontinuity with the same closer vertex already exists */
				if ((memorised_undiscovered_visible.sensor - navpoint).length_sq() < max_distance * max_distance) {
					return true;
				}
			}

			return false;
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
	};
}