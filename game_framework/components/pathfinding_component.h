#pragma once
#include "entity_system/component.h"
#include "math/vec2d.h"

#include "visibility_component.h"

namespace components {
	struct pathfinding : public augs::entity_system::component {
		typedef std::pair<augs::vec2<>, augs::vec2<>> edge;

		pathfinding() : favor_velocity_parallellness(false), enable_backtracking(true), target_offset(0.f), distance_navpoint_hit(0.f), is_exploring(false), rotate_navpoints(0.f) {}

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

		struct pathfinding_session {
			augs::vec2<> target, navigate_to;

			struct navigation_vertex {
				augs::vec2<> location, sensor;
			};

			std::vector<navigation_vertex> discovered_vertices, undiscovered_vertices;
			float temporary_ignore_discontinuities_shorter_than;
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

		bool is_still_pathfinding() const {
			return !session_stack.empty();
		}

		bool is_still_exploring() const {
			return is_exploring;
		}

		void clear_pathfinding_info() {
			session_stack.clear();
			is_exploring = false;
		}
	};
}