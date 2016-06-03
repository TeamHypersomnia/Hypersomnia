#include "pathfinding_component.h"

namespace components {
	pathfinding::pathfinding_session& pathfinding::session() {
		return session_stack.back();
	}

	void pathfinding::start_pathfinding(vec2 target) {
		stop_and_clear_pathfinding();
		session_stack.push_back(pathfinding_session());
		session().target = target;
		session().temporary_ignore_discontinuities_shorter_than = starting_ignore_discontinuities_shorter_than;
		is_exploring = false;
	}

	void pathfinding::start_exploring() {
		stop_and_clear_pathfinding();
		session_stack.push_back(pathfinding_session());
		is_exploring = true;
	}

	void pathfinding::reset_persistent_navpoint() {
		session().persistent_navpoint_set = false;
	}

	void pathfinding::stop_and_clear_pathfinding() {
		session_stack.clear();
		is_exploring = false;
	}

	void pathfinding::restart_pathfinding() {
		session_stack.resize(1);
		session().discovered_vertices.clear();
		session().undiscovered_visible.clear();
		session().undiscovered_vertices.clear();
		session().persistent_navpoint_set = false;
	}

	const pathfinding::pathfinding_session& pathfinding::session() const {
		return session_stack.back();
	}

	vec2 pathfinding::get_current_navigation_point() const {
		return session().navigate_to;
	}

	vec2 pathfinding::get_current_target() const {
		return session().target;
	}

	bool pathfinding::is_still_pathfinding() const {
		return !session_stack.empty();
	}

	bool pathfinding::is_still_exploring() const {
		return is_exploring;
	}

	bool pathfinding::exists_through_undiscovered_visible(vec2 navpoint, float max_distance) const {
		for (auto& memorised_undiscovered_visible : session().undiscovered_visible) {
			/* if a discontinuity with the same closer vertex already exists */
			if ((memorised_undiscovered_visible.sensor - navpoint).length_sq() < max_distance * max_distance) {
				return true;
			}
		}

		return false;
	}
}