#include "pathfinding_system.h"
#include "game/detail/physics/physics_queries.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/for_each_entity.h"

#include "game/inferred_caches/physics_world_cache.h"

#include "game/components/pathfinding_component.h"
#include "game/components/shape_polygon_component.h"
#include "game/components/fixtures_component.h"
#include "game/messages/visibility_information.h"

#include "game/cosmos/entity_handle.h"
#include "game/detail/physics/physics_scripts.h"
#include "game/messages/visibility_information.h"
#include "game/debug_drawing_settings.h"

void pathfinding_system::advance_pathfinding_sessions(const logic_step step) {
#if TODO_PATHFINDING
	auto& cosm = step.get_cosmos();
	const auto si = cosm.get_si();
	const auto& settings = cosm.get_common_significant().pathfinding;

	const auto get_world_vertices = [&](
		const const_entity_handle subject, 
		const bool meters = true,
		const int fixture_num = 0
	) {
		std::vector<b2Vec2> output;

		const auto b = subject.get<components::rigid_body>();

		const auto& verts = subject.get<invariants::shape_polygon>().shape.convex_polys[fixture_num];

		/* for every vertex in given fixture's shape */
		for (auto& v : verts) {
			auto position = b.get_position();
			/* transform angle to degrees */
			auto rotation = b.get_degrees();

			/* transform vertex to current entity's position and rotation */
			vec2 out_vert = vec2(v).rotate(rotation) + position;

			if (meters) {
				out_vert = si.get_meters(out_vert);
			}

			output.push_back(b2Vec2(out_vert));
		}

		return output;
	};

	/* prepare epsilons to be used later, just to make the notation more clear */
	const float epsilon_distance_visible_point_sq = settings.epsilon_distance_visible_point * settings.epsilon_distance_visible_point;
	
	/* we'll need a reference to physics system for raycasting */
	const physics_world_cache& physics = cosm.get_solvable_inferred().physics;

	auto& lines = DEBUG_LOGIC_STEP_LINES;

	cosm.for_each_having<components::pathfinding>(
		[&](const auto it) {
		/* get necessary components */
			static_assert(
				always_false_v<decltype(it)>, 
				"Not implemented. Look for TODO_PERFORMANCE in visibility system and overall improve the algorithm for determining visibility. Preferably, use one with a better complexity."
			);

			auto& pathfinding = it.template get<components::pathfinding>();
			const auto& transform = it.get_logic_transform() + pathfinding.eye_offset;
			const auto body = it.template get<components::rigid_body>();

			if (!body.is_constructed()) {
				return;
			}

			/* check if we request pathfinding at the moment */
			if (!pathfinding.session_stack.empty()) {
				/* get visibility information */
				auto& vision = step.transient.calculated_visibility[it];
				
				std::vector<pathfinding_navigation_vertex> undiscovered_visible;

				/* proceed only if the session is not degenerate */
				if (!vision.edges.empty() && !vision.discontinuities.empty()) {
					if (pathfinding.force_touch_sensors) {
						for (auto& vertex_hit : vision.vertex_hits) {
							messages::visibility_information_response::discontinuity new_discontinuity;
							new_discontinuity.edge_index = vertex_hit.first;
							new_discontinuity.points.first = vertex_hit.second;
							new_discontinuity.is_boundary = false;
							/* rest is not worth filling so proceed */

							if (vertex_hit.second.y >= transform.pos.y) {
								vision.discontinuities.push_back(new_discontinuity);
							}
						}

						vision.vertex_hits.clear();
					}

					//vision.ignore_discontinuities_shorter_than = pathfinding.session().temporary_ignore_discontinuities_shorter_than;

					/* save all fully visible vertices as discovered */
					for (const auto& visible_vertex : vision.vertex_hits) {
						bool this_visible_vertex_is_already_memorised = false;

						//auto pathfinding_session::* location = 
						//		pathfinding.force_touch_sensors ? 
						//	&pathfinding_session::undiscovered_vertices :
						//&pathfinding_session::discovered_vertices;

						for (auto& memorised : pathfinding.session().discovered_vertices) {
							/* if a similiar discovered vertex exists */
							if (memorised.location.compare(visible_vertex.second, settings.epsilon_distance_the_same_vertex)) {
								this_visible_vertex_is_already_memorised = true;
								/* overwrite the location just in case */
								memorised.location = visible_vertex.second;
								break;
							}
						}

						/* save if unique */
						if (!this_visible_vertex_is_already_memorised) {
							pathfinding_navigation_vertex vert;
							vert.location = visible_vertex.second;
							pathfinding.session().discovered_vertices.push_back(vert);
						}
					}



					/* save all new discontinuities from visibility */
					for (const auto& disc : vision.discontinuities) {
						if (disc.is_boundary) continue;

						bool this_discontinuity_is_already_memorised = false;
						bool this_discontinuity_is_already_discovered = false;

						pathfinding_navigation_vertex vert;

						for (auto& memorised_undiscovered : pathfinding.session().undiscovered_vertices) {
							/* if a discontinuity with the same closer vertex already exists */
							if (memorised_undiscovered.location.compare(disc.points.first, settings.epsilon_distance_the_same_vertex)) {
								this_discontinuity_is_already_memorised = true;
								vert = memorised_undiscovered;
								//memorised_undiscovered.location = disc.points.first;
								break;
							}
						}

						for (auto& memorised_discovered : pathfinding.session().discovered_vertices) {
							/* if a discontinuity with the same closer vertex already exists */
							if (memorised_discovered.location.compare(disc.points.first, settings.epsilon_distance_the_same_vertex)) {
								this_discontinuity_is_already_discovered = true;
								memorised_discovered.location = disc.points.first;
								break;
							}
						}

						vert.location = disc.points.first;

						/* if it is unique, push it */
						if (!this_discontinuity_is_already_memorised && !this_discontinuity_is_already_discovered) {

							/* get the associated edge to prepare a relevant sensor */
							auto associated_edge = vision.edges[disc.edge_index];

							/* get the direction the sensor will be going to */
							vec2 sensor_direction;

							bool degenerate = false;

							/* if the first vertex of the edge matches the location */
							if (associated_edge.first.compare(vert.location))
								sensor_direction = associated_edge.first - associated_edge.second;
							/* if it is the second one */
							else if (associated_edge.second.compare(vert.location))
								sensor_direction = associated_edge.second - associated_edge.first;
							/* should never happen, degenerate edge */
							else {
								degenerate = true;
							}
							if (!degenerate) {
								/* rotate a bit to prevent non-reachable sensors */
								float rotation = pathfinding.rotate_navpoints;
								if (disc.winding == disc.LEFT) rotation = -rotation;
								sensor_direction.rotate(rotation);
								//sensor_direction = transform.pos - vert.location;
								sensor_direction.normalize();

								vert.sensor = vert.location + sensor_direction * pathfinding.target_offset;

								/* if this sensor overlaps anything, discard it */
								std::vector<vec2> sensor_polygon = {
									sensor_direction * 10 + vert.location - sensor_direction.perpendicular_cw() * 4,
									sensor_direction * 10 + vert.location - sensor_direction.perpendicular_cw() * 4 + sensor_direction * pathfinding.target_offset,
									sensor_direction * 10 + vert.location + sensor_direction.perpendicular_cw() * 4 + sensor_direction * pathfinding.target_offset,
									sensor_direction * 10 + vert.location + sensor_direction.perpendicular_cw() * 4
								};

								bool was_hit = false;

								physics.for_each_intersection_with_polygon(
									cosm.get_si(),
									sensor_polygon, 
									pathfinding.filter, 
									([&](const auto fix, auto, auto) {
										if (cosm[get_body_entity_that_owns(fix)] != it) {
											was_hit = true;
											return callback_result::ABORT;
										}
										else {
											return callback_result::CONTINUE;
										}
									})
								);

								if (was_hit) {
									constexpr auto rays_count = 50;

									vert.sensor = physics.push_away_from_walls(
										si,
										vert.sensor, 
										pathfinding.target_offset, 
										rays_count,
										pathfinding.filter, 
										it
									);

									pathfinding.session().undiscovered_vertices.push_back(vert);
								}
							}
						}

						if (!this_discontinuity_is_already_discovered)
							undiscovered_visible.push_back(vert);
					}

				}

				/* mark as visible vertices such that:
					a) there is a memorised discovered vertex that is epsilon-close to it
					b) sensor's distance from the the body is less than distance_navpoint_hit 
				*/

				/* prepare body polygon to test for overlaps */
				b2PolygonShape body_poly;
				auto verts = get_world_vertices(it);
				body_poly.Set(verts.data(), static_cast<int32>(verts.size()));

				/* for every undiscovered navigation point */
				auto& undiscs = pathfinding.session().undiscovered_vertices;
				undiscs.erase(std::remove_if(undiscs.begin(), undiscs.end(), [&](const pathfinding_navigation_vertex& nav){

					/* if we want to force the entity to touch the sensors, we can't discard undiscovered vertices only by 
						saying that there exists a discovered vertex (which is discovered only because it is fully visible)
					*/
					//if (!pathfinding.force_touch_sensors) {
						/* find epsilon-close discovered vertices */
						for (auto& memorised_discovered : pathfinding.session().discovered_vertices)
							/* if a similiar discovered vertex exists */
						if (memorised_discovered.location.compare(nav.location, settings.epsilon_distance_the_same_vertex))
								return true;
					//}
					
					if (pathfinding.mark_touched_as_discovered) {
						/* prepare edge shape for sensor to test for overlaps */
						b2EdgeShape sensor_edge;
						sensor_edge.Set(si.get_meters(b2Vec2(nav.location)), si.get_meters(b2Vec2(nav.sensor)));

						/* prepare null transform, both bodies are already in the same frame of reference */
						b2Transform null_transform;
						null_transform.SetIdentity();

						/* if shortest distance between body and sensor fits in distance_navpoint_hit */
						if (b2TestOverlap(&sensor_edge, 0, &body_poly, 0, null_transform, null_transform, si.get_meters(pathfinding.distance_navpoint_hit))) {
							/* save this sensor in discovered ones and return true to remove it from the undiscovered */
							pathfinding.session().discovered_vertices.push_back(nav);
							return true;
						}
					}

					return false;
				}), undiscs.end());

				/* now for the actual pathfinding routine */

				/* helpful lambda */
				auto is_point_visible = [&](vec2 from, vec2 point, b2Filter& filter){
					bool visibility_condition_fulfilled = true;
					
					//if (pathfinding.target_visibility_condition)
					//	visibility_condition_fulfilled = pathfinding.target_visibility_condition(it, from, point);

					if (visibility_condition_fulfilled) {
						const auto line_of_sight = physics.ray_cast_px(
							cosm.get_si(), 
							from, 
							point, 
							filter
						);

						return (!line_of_sight.hit || (line_of_sight.intersection - point).length_sq() < epsilon_distance_visible_point_sq);
					}
					else return false;
				};

				/* we are sure here that session stack has at least 1 session
					we drop secondary sessions whose targets are visible
				*/
				if (pathfinding.enable_session_rollbacks && pathfinding.session_stack.size() >= 2) {
					for (auto old_session = pathfinding.session_stack.begin(); old_session != pathfinding.session_stack.end(); ++old_session) {
						/*  check if there's a line of sight to any of the old targets
						if there's a line of sight to "navigate_to" it will be visible as target to the newer session
						and we either way also handle the current session's target so nothing is missing here
						*/

						/* if we're exploring, we have no target in the first session */
						if (pathfinding.is_exploring && old_session == pathfinding.session_stack.begin())
							continue;

						if (body.test_point((*old_session).target) ||
							is_point_visible(transform.pos, (*old_session).target, pathfinding.filter)) {
								/* if there is, roll back to this session */
								pathfinding.session() = (*old_session);
								
								/* if it is the first session, we don't want to erase it since we still need to reach the target */
								if (old_session == pathfinding.session_stack.begin())
									++old_session;

								/* drop unnecessary sessions */
								pathfinding.session_stack.erase(old_session, pathfinding.session_stack.end());
								break;
						}
					}
				}

				/* if we're exploring, we have no target in the first session */
				if (!pathfinding.is_exploring && pathfinding.session_stack.size() == 1) {
					/* if the target is inside body, it's already found */
					if (body.test_point(pathfinding.session().target)) {
						/* done, target found */
						pathfinding.stop_and_clear_pathfinding();
						return;
					}

					/* check if there's a line of sight */
					if (is_point_visible(transform.pos, pathfinding.session().target, pathfinding.filter)) {
						/* if there is, navigate directly to target */

						pathfinding.session().discovered_vertices.clear();
						pathfinding.session().undiscovered_vertices.clear();

						pathfinding.session().navigate_to = pathfinding.session().target;
						return;
					}
				}

				/* if it is the last session but there's no line of sight,
					or it is not the last session but it was not dropped from the loop which means there's no line of sight to target,
					pick the best navigation candidate

					if we're exploring, pick only visible undiscovered vertices not to get stuck between two nodes
				*/

				auto& vertices = //(pathfinding.is_exploring && pathfinding.session_stack.size() == 1 && !undiscovered_visible.empty()) ?
				//undiscovered_visible : 
				pathfinding.session().undiscovered_vertices;

				/* save only for queries within the function "exists_among_undiscovered_visible" */
				pathfinding.session().undiscovered_visible = undiscovered_visible;

				if (DEBUG_DRAWING.draw_undiscovered_locations) {
					for (auto& disc : vertices)
						lines.emplace_back(disc.location, disc.sensor, rgba(0, 127, 255, 255));

					for (auto& disc : pathfinding.session().discovered_vertices)
						//if(disc.sensor.is_nonzero())
						lines.emplace_back(disc.location, disc.location + vec2(0, pathfinding.target_offset), rgba(0, 255, 0, 255));
				}

				if (!vertices.empty()) {
					bool persistent_navpoint_found = false;

					pathfinding_navigation_vertex current_target;

					if (pathfinding.force_persistent_navpoints) {
						if (pathfinding.session().persistent_navpoint_set) {
							for (auto& v : vertices) {
								if (v.sensor.compare(pathfinding.session().persistent_navpoint.sensor, settings.epsilon_distance_the_same_vertex)) {
									persistent_navpoint_found = true;
									break;
								}
							}
						}
					}

					if (persistent_navpoint_found) {
						current_target = pathfinding.session().persistent_navpoint;
					}
					else {
						vec2 unit_vel = body.get_velocity();
						unit_vel.normalize();
						
						auto local_minimum_predicate = [&pathfinding, &transform, unit_vel](const pathfinding_navigation_vertex& a,
							const pathfinding_navigation_vertex& b) {

							/* if we're exploring, we have no target in the first session */
							if (pathfinding.is_exploring && pathfinding.session_stack.size() == 1) {
								if (pathfinding.favor_velocity_parallellness) {
									float parallellness_a = 0.f;
									float parallellness_b = 0.f;

									if (pathfinding.custom_exploration_hint.enabled) {
										vec2 compared_dir = (pathfinding.custom_exploration_hint.target - pathfinding.custom_exploration_hint.origin).normalize();
										parallellness_a = (a.location - pathfinding.custom_exploration_hint.origin).normalize().dot(compared_dir);
										parallellness_b = (b.location - pathfinding.custom_exploration_hint.origin).normalize().dot(compared_dir);
									}
									else {
										parallellness_a = (a.location - transform.pos).normalize().dot(unit_vel);
										parallellness_b = (b.location - transform.pos).normalize().dot(unit_vel);
									}

									return parallellness_a > parallellness_b;
								}
								else if (pathfinding.custom_exploration_hint.enabled)
									return (a.location - pathfinding.custom_exploration_hint.origin).length_sq() < (b.location - pathfinding.custom_exploration_hint.origin).length_sq();
								else return (a.location - transform.pos).length_sq() < (b.location - transform.pos).length_sq();
							}

							auto dist_a = (a.location - pathfinding.session().target).length_sq() + (a.location - transform.pos).length_sq();
							auto dist_b = (b.location - pathfinding.session().target).length_sq() + (b.location - transform.pos).length_sq();
							return dist_a < dist_b;
						};

						bool first_priority_navpoint_found = false;

						//if (pathfinding.first_priority_navpoint_check) {
						//	std::vector<pathfinding_navigation_vertex> first_priority_candidates;
						//
						//	for (auto& v : vertices) {
						//		try {
						//			/* arguments: subject, transform, navpoint 
						//				returns true or false
						//			*/
						//			if (pathfinding.first_priority_navpoint_check(it, transform.pos, v.sensor)) {
						//				first_priority_candidates.push_back(v);
						//			}
						//		}
						//		catch (const std::exception& compilation_error) {
						//			LOG(compilation_error.what());
						//		}
						//	}
						//
						//	if (!first_priority_candidates.empty()) {
						//		/* find discontinuity that is closest to the target */
						//		current_target = (*std::min_element(first_priority_candidates.begin(), first_priority_candidates.end(), local_minimum_predicate));
						//
						//		first_priority_navpoint_found = true;
						//	}
						//}

						if (!first_priority_navpoint_found) 
							/* find discontinuity that is closest to the target */
							current_target = (*std::min_element(vertices.begin(), vertices.end(), local_minimum_predicate));

						if (pathfinding.force_persistent_navpoints) {
							pathfinding.session().persistent_navpoint_set = true;
							pathfinding.session().persistent_navpoint = current_target;
						}
					}

					/* extract the closer vertex, condition to faciliate debug */
					if (current_target.sensor != pathfinding.session().navigate_to)
						pathfinding.session().navigate_to = current_target.sensor;


					if (DEBUG_DRAWING.draw_undiscovered_locations) {
						lines.emplace_back(transform.pos, current_target.sensor, rgba(255, 255, 0, 255));
						lines.emplace_back(transform.pos, pathfinding.session().target, rgba(255, 0, 0, 255));
					}

					bool rays_hit = false;
					/* extract all transformed vertices of the subject's original model, false means we want pixels */
					auto subject_verts = get_world_vertices(it, false);
					subject_verts.clear();
					subject_verts.push_back(b2Vec2(transform.pos));

					for (auto& subject_vert : subject_verts) {
						if (
							//is_point_visible(subject_vert, current_target.location, pathfinding.filter) ||
							is_point_visible(subject_vert, current_target.sensor, pathfinding.filter)
							) {

							/* assume for now that the rays DID hit the navpoint */
							rays_hit = true;

							/* now see if the navpoint can be seen through marked non-walkable areas
								prepare raycast data
							*/
							b2RayCastOutput output;
							b2RayCastInput input;
							input.maxFraction = 1.0;

							for (auto& marked : vision.marked_holes) {
								/* prepare raycast subject */
								b2EdgeShape marked_hole;
								marked_hole.Set(b2Vec2(marked.first), b2Vec2(marked.second));

								input.p1 = subject_vert;
								input.p2 = b2Vec2(current_target.location);

								/* we don't need to transform edge or ray since they are in the same space
								but we have to prepare dummy b2Transform as argument for b2EdgeShape::RayCast
								*/
								b2Transform null_transform;
								null_transform.SetIdentity();

								if (marked_hole.RayCast(&output, input, null_transform, 0)) {
									rays_hit = false;
									break;
								}

								input.p2 = b2Vec2(current_target.sensor);

								if (marked_hole.RayCast(&output, input, null_transform, 0)) {
									rays_hit = false;
									break;
								}
							}
						}
					}

					/* if we can see it, navigate there */
					if (body.test_point(current_target.location) ||
						body.test_point(current_target.sensor) ||
						rays_hit
						) {
					}
					/* else start new navigation session */
					else {
						if (pathfinding.enable_backtracking) {
							vec2 new_target = pathfinding.session().navigate_to;
							pathfinding.session_stack.push_back(pathfinding_session());
							pathfinding.session().target = new_target;
							pathfinding.session().temporary_ignore_discontinuities_shorter_than = pathfinding.starting_ignore_discontinuities_shorter_than;
						}
					}

				}
				else {
					/* something went wrong, let's begin again */
					//if (pathfinding.session_stack.size() == 1) {
						pathfinding.session().discovered_vertices.clear();
						pathfinding.session().undiscovered_vertices.clear();
						pathfinding.session().persistent_navpoint_set = false;
						pathfinding.session().undiscovered_visible.clear();
						//pathfinding.session().temporary_ignore_discontinuities_shorter_than /= 1.5f;
					//}
					//pathfinding.session_stack.resize(1);
				}
			}
		}
	);
#else
	(void)step;
#endif
}