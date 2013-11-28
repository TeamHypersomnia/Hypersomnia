#pragma once
#include "entity_system/component.h"
#include "math/vec2d.h"

#include "visibility_component.h"

namespace components {
	struct pathfinding : public augmentations::entity_system::component {
		typedef std::pair<augmentations::vec2<>, augmentations::vec2<>> edge;

		pathfinding() : enable_backtracking(true), target_offset(0.f), distance_navpoint_hit(0.f), is_exploring(false), rotate_navpoints(0.f) {}

		bool enable_backtracking;
		bool is_exploring;

		float target_offset;
		float rotate_navpoints;
		float distance_navpoint_hit;
		float starting_ignore_discontinuities_shorter_than;

		struct pathfinding_session {
			augmentations::vec2<> target, navigate_to;

			struct navigation_vertex {
				augmentations::vec2<> location, sensor;
			};

			std::vector<navigation_vertex> discovered_vertices, undiscovered_vertices;
			float temporary_ignore_discontinuities_shorter_than;

			//std::vector<edge> visible_walls, undiscovered_walls;
			//std::vector<visibility::discontinuity> undiscovered_discontinuities;
		};

		std::vector <pathfinding_session> session_stack;

		pathfinding_session& session() {
			return session_stack.back();
		}

		void start_pathfinding(augmentations::vec2<> target) {
			is_exploring = false;
			clear_pathfinding_info();
			session_stack.push_back(pathfinding_session());
			session().target = target;
			session().temporary_ignore_discontinuities_shorter_than = starting_ignore_discontinuities_shorter_than;
		}

		void start_exploring() {
			is_exploring = true;
			clear_pathfinding_info();
			session_stack.push_back(pathfinding_session());
		}

		augmentations::vec2<> get_current_navigation_target() {
			return session().navigate_to;
		}

		augmentations::vec2<> get_current_target() {
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