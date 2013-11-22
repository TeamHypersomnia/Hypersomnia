#pragma once
#include "entity_system/component.h"
#include "math/vec2d.h"

#include "visibility_component.h"

namespace components {
	struct pathfinding : public augmentations::entity_system::component {
		typedef std::pair<augmentations::vec2<>, augmentations::vec2<>> edge;

		pathfinding() : enable_backtracking(true), target_offset(0.f), distance_navpoint_hit(0.f) {}

		bool enable_backtracking;

		float target_offset;
		float distance_navpoint_hit;

		struct pathfinding_session {
			augmentations::vec2<> target, navigate_to;

			struct navigation_vertex {
				augmentations::vec2<> location, sensor;
			};

			std::vector<navigation_vertex> discovered_vertices, undiscovered_vertices;

			//std::vector<edge> visible_walls, undiscovered_walls;
			//std::vector<visibility::discontinuity> undiscovered_discontinuities;
		};

		std::vector <pathfinding_session> session_stack;

		pathfinding_session& session() {
			return session_stack.back();
		}

		void start_pathfinding(augmentations::vec2<> target) {
			clear_pathfinding_info();
			session_stack.push_back(pathfinding_session());
			session().target = target;
		}

		augmentations::vec2<> get_current_navigation_target() {
			return session().navigate_to;
		}

		bool is_still_pathfinding() const {
			return !session_stack.empty();
		}

		void clear_pathfinding_info() {
			session_stack.clear();
		}
	};
}