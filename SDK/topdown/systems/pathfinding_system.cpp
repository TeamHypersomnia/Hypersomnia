#include "stdafx.h"
#include "pathfinding_system.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "render_system.h"
#include "physics_system.h"
#include "../resources/render_info.h"

pathfinding_system::pathfinding_system() : draw_memorised_walls(false), draw_undiscovered(false), ignore_discontinuities_shorter_than(1.f) {}

void pathfinding_system::process_entities(world& owner) {
	/* prepare epsilons to be used later, just to make the notation more clear */
	const float ignore_discontinuities_shorter_than_sq = ignore_discontinuities_shorter_than * ignore_discontinuities_shorter_than;
	const float epsilon_distance_visible_point_sq = epsilon_distance_visible_point * epsilon_distance_visible_point;

	/* we'll need a reference to physics system for raycasting */
	physics_system& physics = owner.get_system<physics_system>();
	/* we'll need a reference to render system for debug drawing */
	render_system& render = owner.get_system<render_system>();

	for (auto it : targets) {
		/* get AI data and position of the entity */
		auto& visibility = it->get<components::visibility>();
		auto& pathfinding = it->get<components::pathfinding>();
		auto& transform = it->get<components::transform>().current;
		auto body = it->get<components::physics>().body;

		/* check if we request pathfinding */
		if (pathfinding.is_finding_a_path) {
			/* get visibility information */
			auto& vision = visibility.get_layer(components::visibility::DYNAMIC_PATHFINDING);

			/* save all undiscovered walls from visibility */
			//for (auto& new_wall : vision.visible_walls) {
			//	bool such_a_wall_exists = false;
			//	/* compare this wall with already memorised walls */
			//	for (auto& memorised_wall : pathfinding.session.visible_walls) {
			//		/* if any memorised wall is almost equal to the candidate */
			//		if (( memorised_wall.first.compare_sq(new_wall.first,  epsilon_max_segment_difference) && 
			//			  memorised_wall.second.compare_sq(new_wall.second, epsilon_max_segment_difference))
			//			||
			//			(memorised_wall.first.compare_sq(new_wall.second, epsilon_max_segment_difference) &&
			//			memorised_wall.second.compare_sq(new_wall.first, epsilon_max_segment_difference))
			//			) {
			//				/* there's no need to push back the new one */
			//				such_a_wall_exists = true;
			//				break;
			//		}
			//	}
			//
			//	if (!such_a_wall_exists) 
			//		pathfinding.session.visible_walls.push_back(new_wall);
			//}
			for (auto& visible_vertex : vision.vertex_hits) {
				bool this_visible_vertex_is_already_memorised = false;

				for (auto& memorised_discovered : pathfinding.session.discovered_vertices) {
					/* if a discontinuity with the same closer vertex already exists */
					if ((memorised_discovered.location - visible_vertex).length() < 3.f) {
						this_visible_vertex_is_already_memorised = true;
						break;
					}
				}

				if (!this_visible_vertex_is_already_memorised) {
					components::pathfinding::pathfinding_session::navigation_vertex vert;
					vert.location = visible_vertex;
					pathfinding.session.discovered_vertices.push_back(vert);
				}
			}

			/* save all new discontinuities from visibility */
			for (auto& disc : vision.discontinuities) {
				if ((disc.points.second - disc.points.first).length_sq() < ignore_discontinuities_shorter_than_sq)
					continue;

				bool this_discontinuity_is_already_memorised = false;

				for (auto& memorised_undiscovered : pathfinding.session.undiscovered_vertices) {
					/* if a discontinuity with the same closer vertex already exists */
					if ((memorised_undiscovered.location - disc.points.first).length() < 3.f) {
						this_discontinuity_is_already_memorised = true;
						break;
					}
				}

				for (auto& memorised_discovered : pathfinding.session.discovered_vertices) {
					/* if a discontinuity with the same closer vertex already exists */
					if ((memorised_discovered.location - disc.points.first).length() < 3.f) {
						this_discontinuity_is_already_memorised = true;
						break;
					}
				}

				/* if it is unique, push it */
				if (!this_discontinuity_is_already_memorised) {
					components::pathfinding::pathfinding_session::navigation_vertex vert;
					vert.location = disc.points.first;
					/* push the sensor a bit further so the body doesn't stop not seeing another targets */
					vert.sensor = vert.location + (disc.points.second - vert.location).normalize() * pathfinding.target_offset;
					pathfinding.session.undiscovered_vertices.push_back(vert);
				}
			}

			/* mark vertices whose sensors touch the body as visited */

			auto& undiscs = pathfinding.session.undiscovered_vertices;
			undiscs.erase(std::remove_if(undiscs.begin(), undiscs.end(), [&body, &pathfinding](const components::pathfinding::pathfinding_session::navigation_vertex& nav){
				//b2CircleShape b2circle;
				
				if (body->TestPoint(nav.sensor * PIXELS_TO_METERSf)) {
					pathfinding.session.discovered_vertices.push_back(nav);
					return true;
				}
				return false;
			}), undiscs.end());

			/* delete all discontinuities that fall into any of fully visible walls */
			//auto& discs = pathfinding.session.undiscovered_discontinuities;
			//discs.erase(std::remove_if(discs.begin(), discs.end(), [&pathfinding, this](const components::visibility::discontinuity& d){
			//	bool this_discontinuity_is_discovered = false;
			//
			//	/* for all visible walls */
			//	for (auto& wall : pathfinding.session.visible_walls) {
			//		/* check if the further vertex is near enough the segment representing the wall */
			//		if (d.points.second.distance_from_segment_sq(wall.first, wall.second) < epsilon_max_segment_difference) {
			//			this_discontinuity_is_discovered = true;
			//			break;
			//		}
			//	}
			//
			//	return this_discontinuity_is_discovered;
			//}), discs.end());

			/* debug drawing */
			//if (draw_memorised_walls) {
			//	for (auto& wall : pathfinding.session.visible_walls) {
			//		render.lines.push_back(render_system::debug_line(wall.first, wall.second, graphics::pixel_32(0, 255, 0, 255)));
			//	}
			//}

			/* now for the actual pathfinding routine */

			/* helpful lambda */
			auto& is_point_visible = [&physics, epsilon_distance_visible_point_sq](vec2<> from, vec2<> point, b2Filter& filter){
				auto line_of_sight = physics.ray_cast_px(from, point, &filter);
				return (!line_of_sight.hit || (line_of_sight.intersection - point).length_sq() < epsilon_distance_visible_point_sq);
			};

			bool target_inside_body = body->TestPoint(pathfinding.session.target * PIXELS_TO_METERSf);

			if (pathfinding.session_stack.empty() && target_inside_body) {
				pathfinding.is_finding_a_path = false;
			}
			else {
				/* first check if there's a line of sight */
				if (target_inside_body || is_point_visible(transform.pos, pathfinding.session.target, vision.filter)) {
					/* if we're actually navigating to a secondary target */
					if (!pathfinding.session_stack.empty()) {
						/* roll back to the previous session */
						const auto top = pathfinding.session_stack.end() - 1;
						pathfinding.session = *top;
						pathfinding.session_stack.erase(top);
					}
					else pathfinding.session.navigate_to = pathfinding.session.target;
				}
				else {
					auto& vertices = pathfinding.session.undiscovered_vertices;

					if (draw_undiscovered) {
						for (auto& disc : vertices)
							render.lines.push_back(render_system::debug_line(disc.location, disc.sensor, graphics::pixel_32(0, 127, 255, 255)));
					}

					if (!vertices.empty()) {
						/* find discontinuity that is closest to the target */
						auto& local_minimum_discontinuity = (*std::min_element(vertices.begin(), vertices.end(),
							[&pathfinding, &transform](const components::pathfinding::pathfinding_session::navigation_vertex& a,
							const components::pathfinding::pathfinding_session::navigation_vertex& b) {
								auto dist_a = (a.location - pathfinding.session.target).length_sq() + (a.location - transform.pos).length_sq();
								auto dist_b = (b.location - pathfinding.session.target).length_sq() + (b.location - transform.pos).length_sq();
								return dist_a < dist_b;
						}));

						/* extract the closer vertex, condition to faciliate debug */
						if (local_minimum_discontinuity.sensor != pathfinding.session.navigate_to)
							pathfinding.session.navigate_to = local_minimum_discontinuity.sensor;

						/* if we can see it, navigate there */
						if (body->TestPoint(local_minimum_discontinuity.location * PIXELS_TO_METERSf) ||
							is_point_visible(transform.pos, local_minimum_discontinuity.location, vision.filter)) {
						}
						/* else start new navigation session */
						else {
							if (pathfinding.enable_backtracking) {
								vec2<> new_target = pathfinding.session.navigate_to;

								pathfinding.session_stack.push_back(pathfinding.session);
								pathfinding.session = components::pathfinding::pathfinding_session();
								pathfinding.session.target = new_target;
							}
						}

					}
				}
			}
		}
	}
}