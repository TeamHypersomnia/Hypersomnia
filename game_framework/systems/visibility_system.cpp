#include "stdafx.h"
#include "visibility_system.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "physics_system.h"
#include "render_system.h"

#include "../resources/render_info.h"
#include "../game/body_helper.h"

#include <limits>
#include <set>

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

/*
source:
http://stackoverflow.com/questions/16542042/fastest-way-to-sort-vectors-by-angle-without-actually-computing-that-angle
*/
float comparable_angle(vec2<> diff) {
	return sgn(diff.y) * (
		1 - (diff.x / (std::abs(diff.x) + std::abs(diff.y)))
		);
}

visibility_system::visibility_system() : draw_cast_rays(false), draw_triangle_edges(true), draw_discontinuities(false), draw_visible_walls(false) {}

int components::visibility::layer::get_num_triangles() {
	return edges.size();
}

components::visibility::discontinuity* components::visibility::layer::get_discontinuity(int n) {
	for (auto& disc : discontinuities)
		if (disc.edge_index == n)
			return &disc;

	return nullptr;
}

components::visibility::triangle components::visibility::layer::get_triangle(int i, augs::vec2<> origin) {
	components::visibility::triangle tri = { origin + offset, edges[i].first, edges[i].second };
	return tri;
}

void visibility_system::process_entities(world& owner) {
	/* prepare epsilons to be used later, just to make the notation more clear */
	float epsilon_distance_vertex_hit_sq = epsilon_distance_vertex_hit * PIXELS_TO_METERSf;
	float epsilon_threshold_obstacle_hit_meters = epsilon_threshold_obstacle_hit * PIXELS_TO_METERSf;
	epsilon_distance_vertex_hit_sq *= epsilon_distance_vertex_hit_sq;

	/* we'll need a reference to physics system for raycasting */
	physics_system& physics = owner.get_system<physics_system>();
	/* we'll need a reference to render system for debug drawing */
	render_system& render = owner.get_system<render_system>();

	struct ray_input {
		vec2<> targets[2];
	};

	std::vector<std::pair<physics_system::raycast_output, physics_system::raycast_output>> all_ray_outputs;
	std::vector<ray_input> all_ray_inputs;

	for (auto it : targets) {
		/* get AI data and position of the entity */
		auto& visibility = it->get<components::visibility>();
		auto& transform = it->get<components::transform>().current;

		auto body = it->get<components::physics>().body;

		/* for every visibility type requested for given entity */
		for (auto& entry : visibility.visibility_layers.raw) {

			/* prepare container for all the vertices that we will cast the ray to */
			struct target_vertex {
				bool is_on_a_bound;
				float angle;
				vec2<> pos;

				bool operator<(const target_vertex& b) {
					return angle < b.angle;
				}
			};

			static std::vector <target_vertex> all_vertices_transformed;
			all_vertices_transformed.clear();

			/* shortcut */
			auto& request = entry.val;

			/* transform entity position to Box2D coordinates and take offset into account */
			vec2<> position_meters = (transform.pos + request.offset) * PIXELS_TO_METERSf;

			/* to Box2D coordinates */
			float vision_side_meters = request.square_side * PIXELS_TO_METERSf;

			/* prepare maximum visibility square */
			b2AABB aabb;
			aabb.lowerBound = position_meters - vision_side_meters / 2;
			aabb.upperBound = position_meters + vision_side_meters / 2;

			rects::ltrb ltrb(aabb.lowerBound.x, aabb.lowerBound.y, aabb.upperBound.x, aabb.upperBound.y);

			auto& push_vertex = [position_meters, ltrb](vec2<> v, bool check_against_aabb){
				/* don't bother if it does not hover the aabb */
				if (check_against_aabb && !ltrb.hover(vec2<>(v))) 
					return;

				target_vertex new_vertex;
				new_vertex.pos = v;

				/* calculate difference vector */
				vec2<> diff = new_vertex.pos - position_meters;
				/*
				compute angle to be compared while sorting
				source:
				http://stackoverflow.com/questions/16542042/fastest-way-to-sort-vectors-by-angle-without-actually-computing-that-angle

				save the angle in pair next to the vertex position, we will then sort the "angle-vertex" pairs by angle */
				new_vertex.angle = comparable_angle(diff);
				new_vertex.is_on_a_bound = !check_against_aabb;

				/* save transformed vertex */
				all_vertices_transformed.push_back(new_vertex);
			};

			/* get list of all fixtures that intersect with the visibility square */
			auto bodies = physics.query_aabb(aabb.lowerBound, aabb.upperBound, &request.filter, it).bodies;

			/* for every fixture that intersected with the visibility square */
			for (auto b : bodies) {
				/* get shape vertices from misc that transforms them to current entity's position and rotation in Box2D space */
				auto verts = topdown::get_transformed_shape_verts(*reinterpret_cast<entity*>(b->GetUserData()));
				/* for every vertex in given fixture's shape */
				for (auto& v : verts) 
					push_vertex(v, true);
			}

			/* extract the actual vertices from visibility AABB to cast rays to */
			b2Vec2 whole_vision [] = {
				aabb.lowerBound,
				aabb.lowerBound + vec2<>(vision_side_meters, 0),
				aabb.upperBound,
				aabb.upperBound - vec2<>(vision_side_meters, 0)
			};

			/* prepare edge shapes given above vertices to cast rays against when no obstacle was hit
			note we lengthen them a bit and add/substract 1.f to avoid undeterministic vertex cases
			*/
			b2EdgeShape bounds[4];
			float moving_epsilon = 1.f * PIXELS_TO_METERSf;
			bounds[0].Set(vec2<>(whole_vision[0]) + vec2<>(-moving_epsilon, 0.f), vec2<>(whole_vision[1]) + vec2<>(moving_epsilon, 0.f));
			bounds[1].Set(vec2<>(whole_vision[1]) + vec2<>(0.f, -moving_epsilon), vec2<>(whole_vision[2]) + vec2<>(0.f, moving_epsilon));
			bounds[2].Set(vec2<>(whole_vision[2]) + vec2<>(moving_epsilon, 0.f), vec2<>(whole_vision[3]) + vec2<>(-moving_epsilon, 0.f));
			bounds[3].Set(vec2<>(whole_vision[3]) + vec2<>(0.f, moving_epsilon), vec2<>(whole_vision[0]) + vec2<>(0.f, -moving_epsilon));

			/* debug drawing of the visibility square */
			if (draw_cast_rays || draw_triangle_edges) {
				render.lines.push_back(render_system::debug_line((vec2<>(whole_vision[0]) + vec2<>(-moving_epsilon, 0.f))*METERS_TO_PIXELSf, (vec2<>(whole_vision[1]) + vec2<>(moving_epsilon, 0.f))*METERS_TO_PIXELSf));
				render.lines.push_back(render_system::debug_line((vec2<>(whole_vision[1]) + vec2<>(0.f, -moving_epsilon))*METERS_TO_PIXELSf, (vec2<>(whole_vision[2]) + vec2<>(0.f, moving_epsilon))*METERS_TO_PIXELSf));
				render.lines.push_back(render_system::debug_line((vec2<>(whole_vision[2]) + vec2<>(moving_epsilon, 0.f))*METERS_TO_PIXELSf, (vec2<>(whole_vision[3]) + vec2<>(-moving_epsilon, 0.f))*METERS_TO_PIXELSf));
				render.lines.push_back(render_system::debug_line((vec2<>(whole_vision[3]) + vec2<>(0.f, moving_epsilon))*METERS_TO_PIXELSf, (vec2<>(whole_vision[0]) + vec2<>(0.f, -moving_epsilon))*METERS_TO_PIXELSf));
			}

			/* raycast through the bounds to add another vertices where the shapes go beyond visibility square */
			for (auto& bound : bounds) {
				/* have to raycast both directions because Box2D ignores the second side of the fixture */
				auto output1 = physics.ray_cast_all_intersections(bound.m_vertex1, bound.m_vertex2, request.filter, it);
				auto output2 = physics.ray_cast_all_intersections(bound.m_vertex2, bound.m_vertex1, request.filter, it);

				/* check for duplicates */
				std::vector<vec2<>> output;

				for (auto& inter : output1) {
					output.push_back(inter.intersection);
				}

				for (auto& v : output2) {
					bool duplicate_found = false;

					for (auto& duplicate : output1) {
						if (v.intersection.compare(duplicate.intersection)) {
							duplicate_found = true;
							break;
						}
					}

					if (!duplicate_found) 
						output.push_back(v.intersection);
				}

				if (draw_cast_rays) 
					render.lines.push_back(render_system::debug_line(METERS_TO_PIXELSf * bound.m_vertex1, METERS_TO_PIXELSf * bound.m_vertex2, graphics::pixel_32(255, 0, 0, 255)));

				for (auto& v : output)
					push_vertex(v, false);
			}

			/* add the visibility square to the vertices that we cast rays to, computing comparable angle in place */
			for (auto& v : whole_vision)
				push_vertex(v, false);

			/* SORT ALL VERTICES BY ANGLE */
			std::sort(all_vertices_transformed.begin(), all_vertices_transformed.end());

			/* by now we have ensured that all_vertices_transformed is non-empty

			debugging:
			red ray - ray that intersected with obstacle, these are ignored
			yellow ray - ray that hit the same vertex
			violet ray - ray that passed through vertex and hit another obstacle
			blue ray - ray that passed through vertex and hit boundary
			*/

			/* double_ray pair for holding both left-epsilon and right-epsilon rays */
			struct double_ray {
				vec2<> first, second;
				bool first_reached_destination, second_reached_destination;

				double_ray() : first_reached_destination(false), second_reached_destination(false) {}
				double_ray(vec2<> first, vec2<> second, bool a, bool b)
					: first(first), second(second), first_reached_destination(a), second_reached_destination(b) {
				}
			};

			/* container for these */
			std::vector<double_ray> double_rays;

			/* helper debugging lambda */
			auto draw_line = [&position_meters, &render](vec2<> point, graphics::pixel_32 col) {
				render.lines.push_back(render_system::debug_line(position_meters * METERS_TO_PIXELSf, point * METERS_TO_PIXELSf, col));
			};

			/* clear per-frame visibility information */
			request.discontinuities.clear();
			request.vertex_hits.clear();
			request.edges.clear();

			all_ray_inputs.clear();
			all_ray_outputs.clear();

			all_ray_inputs.reserve(all_vertices_transformed.size());
			all_ray_outputs.reserve(all_vertices_transformed.size());

			/* for every vertex to cast the ray to */
			for (auto& vertex : all_vertices_transformed) {
				/* create two vectors in direction of vertex with length equal to the half of diagonal of the visibility square
				(majority of rays will SLIGHTLY go beyond visibility square, but that's not important now)
				ray_callbacks[0] and ray_callbacks[1] differ ONLY by an epsilon added / substracted to the angle
				*/

				/* calculate the perpendicular direction to properly apply epsilon_ray_distance_variation */
				vec2<> perpendicular_cw = (vertex.pos - position_meters).normalize().perpendicular_cw();

				vec2<> directions[2] = {
					((vertex.pos - perpendicular_cw * epsilon_ray_distance_variation * PIXELS_TO_METERSf) - position_meters).normalize(),
					((vertex.pos + perpendicular_cw * epsilon_ray_distance_variation * PIXELS_TO_METERSf) - position_meters).normalize()
				};

				vec2<> targets[2] = {
					position_meters + directions[0] * vision_side_meters / 2 * 1.5,
					position_meters + directions[1] * vision_side_meters / 2 * 1.5
				};

				/* clamp the ray to the bound */
				for (auto& bound : bounds) {
					bool continue_checking = true;

					for (int j = 0; j < 2; ++j) {
						auto edge_ray_output = physics.edge_edge_intersection(position_meters, targets[j], bound.m_vertex1, bound.m_vertex2);
						if (edge_ray_output.hit) {
							/* move the target further by epsilon */
							targets[j] = edge_ray_output.intersection + directions[j] * 1.f * PIXELS_TO_METERSf;
							continue_checking = false;
						}
						else {
							bool breakpoint = true;
						}
					}

					if (!continue_checking) break;
				}

				/* cast both rays starting from the player position and ending in targets[x].target,
				ignoring subject entity ("it") completely, save results in ray_callbacks[2] */
				ray_input new_ray_input;
				
				new_ray_input.targets[0] = vertex.is_on_a_bound ? vertex.pos : targets[0];
				new_ray_input.targets[1] = targets[1];
				all_ray_inputs.push_back(new_ray_input);
			}
			
			/* process all raycast inputs at once to improve cache coherency */
			for (int j = 0; j < all_ray_inputs.size(); ++j) {
				all_ray_outputs.push_back(std::make_pair(
					physics.ray_cast(position_meters, all_ray_inputs[j].targets[0], request.filter, it),
					all_vertices_transformed[j].is_on_a_bound ? 
					physics_system::raycast_output() : physics.ray_cast(position_meters, all_ray_inputs[j].targets[1], request.filter, it)
				));
			}

			for (size_t i = 0; i < all_ray_outputs.size(); ++i) {
				physics_system::raycast_output ray_callbacks[2] = { all_ray_outputs[i].first, all_ray_outputs[i].second };
				auto& vertex = all_vertices_transformed[i];

				b2Vec2* from_aabb = nullptr;

				/* if the vertex comes from bounding square, save it and remember about it */
				for (auto& aabb_vert : whole_vision)
					if (vertex.pos == aabb_vert)
						from_aabb = &aabb_vert;

					if (vertex.is_on_a_bound && (!ray_callbacks[0].hit || (ray_callbacks[0].intersection - vertex.pos).length_sq() < epsilon_distance_vertex_hit_sq)) {
					/* if it is a vertex on the boundary, handle it accordingly - interpret it as a new discontinuity (e.g. for pathfinding) */
					components::visibility::discontinuity new_discontinuity;
					new_discontinuity.edge_index = double_rays.size();
					new_discontinuity.points.first = vertex.pos;

					vec2<> actual_normal;// = (ray_callbacks[0].normal / 2;
					vec2<> actual_intersection;// = (+ray_callbacks[1].intersection) / 2;
					new_discontinuity.points.second = ray_callbacks[0].intersection;
					new_discontinuity.normal = actual_normal;

					new_discontinuity.winding = actual_normal.cross(position_meters - actual_intersection) > 0 ?
						components::visibility::discontinuity::RIGHT : components::visibility::discontinuity::LEFT;

					/* if it is clockwise, we take previous edge as subject */
					if (new_discontinuity.winding == components::visibility::discontinuity::RIGHT)
						--new_discontinuity.edge_index;

					new_discontinuity.is_boundary = true;
					//request.discontinuities.push_back(new_discontinuity);
					double_rays.push_back(double_ray(vertex.pos, vertex.pos, true, true));
					if (draw_cast_rays) draw_line(vertex.pos, graphics::pixel_32(255, 255, 0, 255));
				}
				else if (!vertex.is_on_a_bound) {
					/* if we did not intersect with anything */
					//if (!(ray_callbacks[0].hit || ray_callbacks[1].hit)) {
					//	/* we could have cast the ray against bounding square, so if it in fact comes from there */
					//	if (from_aabb)
					//		/* interpret as both rays hit the square */
					//		double_rays.push_back(double_ray(*from_aabb, *from_aabb, true, true));
					//
					//	/* should never happen (but who knows) */
					//	else {
					//		bool breakpoint = true;
					//	}
					//}
					///* both rays intersect with something */
					//else 
					if (ray_callbacks[0].hit && ray_callbacks[1].hit) {
						/* if distance between both intersections and position is less than distance from target to position
						then rays must have intersected with an obstacle BEFORE reaching the vertex, ignoring intersection completely */
						float distance_from_origin = (vertex.pos - position_meters).length();

						if ((ray_callbacks[0].intersection - position_meters).length() + epsilon_threshold_obstacle_hit_meters < distance_from_origin &&
							(ray_callbacks[1].intersection - position_meters).length() + epsilon_threshold_obstacle_hit_meters < distance_from_origin) {
							if (draw_cast_rays) draw_line(vertex.pos, graphics::pixel_32(255, 0, 0, 255));
						}
						/* distance between both intersections fit in epsilon which means ray intersected with the same vertex */
						else if ((ray_callbacks[0].intersection - ray_callbacks[1].intersection).length_sq() < epsilon_distance_vertex_hit_sq) {
							/* interpret it as both rays hit the same vertex
							for maximum accuracy, push the vertex coordinates instead of the actual intersections */

							request.vertex_hits.push_back(vertex.pos * METERS_TO_PIXELSf);
							double_rays.push_back(double_ray(vertex.pos, vertex.pos, true, true));
							if (draw_cast_rays) draw_line(vertex.pos, graphics::pixel_32(255, 255, 0, 255));
						}
						/* we're here so:
						they reached the target or even further (guaranteed by first condition),
						and they are not close to each other (guaranteed by condition 2),
						so either one of them hit the vertex and the second one went further or we have a pathological case
						that both wen't further and still intersected somewhere close - this is something we either way cannot handle and occurs
						when a body is pathologically thin

						so this is the case where the ray is cast at the lateral vertex,
						here we also detect the discontinuity */
						else {
							/* save both intersection points, this is what we introduced "double_ray" pair for */
							double_ray new_double_ray(ray_callbacks[0].intersection, ray_callbacks[1].intersection, false, false);


							/* save new discontinuity */
							components::visibility::discontinuity new_discontinuity;

							/* if the ray that we substracted the epsilon from intersected closer (and thus with the vertex), then the free space is to the right */
							if ((ray_callbacks[0].intersection - vertex.pos).length_sq() <
								(ray_callbacks[1].intersection - vertex.pos).length_sq()) {
								/* it was "first" one that directly reached its destination */
								new_double_ray.first_reached_destination = true;
								new_double_ray.first = vertex.pos;

								/* save discontinuity info and edge_index to associate discontinuity with current edge */
								new_discontinuity.points.first = vertex.pos;
								new_discontinuity.points.second = ray_callbacks[1].intersection;
								new_discontinuity.winding = components::visibility::discontinuity::RIGHT;
								new_discontinuity.edge_index = double_rays.size() - 1;
								if (draw_cast_rays) draw_line(ray_callbacks[1].intersection, graphics::pixel_32(255, 0, 255, 255));
							}
							/* otherwise the free area is to the left */
							else {
								/* it was "second" one that directly reached its destination */
								new_double_ray.second_reached_destination = true;
								new_double_ray.second = vertex.pos;

								new_discontinuity.points.first = vertex.pos;
								new_discontinuity.points.second = ray_callbacks[0].intersection;
								new_discontinuity.winding = components::visibility::discontinuity::LEFT;
								new_discontinuity.edge_index = double_rays.size();
								if (draw_cast_rays) draw_line(ray_callbacks[0].intersection, graphics::pixel_32(255, 0, 255, 255));
							}
							/* save new double ray */
							double_rays.push_back(new_double_ray);
							/* save new discontinuity */
							request.discontinuities.push_back(new_discontinuity);
						}
					}
					/* the case where exactly one of the rays did not hit anything so we cast it against boundaries,
					we also detect discontinuity here */
					else {
						/* for every callback that didn't detect hit (there will be only one) */
						for (size_t k = 0; k < 2; ++k) {
							if (!ray_callbacks[k].hit) {
								/* for every edge from 4 edges forming visibility square */
								for (auto& bound : bounds) {
									auto ray_edge_output = physics.edge_edge_intersection(position_meters, all_ray_inputs[i].targets[k], bound.m_vertex1, bound.m_vertex2);

									/* if we hit against boundaries (must happen for at least 1 of them) */
									if (ray_edge_output.hit) {
										/* prepare new discontinuity data */
										components::visibility::discontinuity new_discontinuity;

										/* compute the actual intersection point from b2RayCastOutput data */
										auto actual_intersection = ray_edge_output.intersection;

										new_discontinuity.points.first = vertex.pos;
										new_discontinuity.points.second = actual_intersection;

										/* if the left-handed ray intersected with boundary and thus the right-handed intersected with an obstacle */
										if (k == 0) {
											new_discontinuity.winding = components::visibility::discontinuity::LEFT;
											new_discontinuity.edge_index = double_rays.size();
											double_rays.push_back(double_ray(actual_intersection, vertex.pos, false, true));
										}
										/* if the right-handed ray intersected with boundary and thus the left-handed intersected with an obstacle */
										else if (k == 1) {
											new_discontinuity.winding = components::visibility::discontinuity::RIGHT;
											new_discontinuity.edge_index = double_rays.size() - 1;
											double_rays.push_back(double_ray(vertex.pos, actual_intersection, true, false));
										}
										request.discontinuities.push_back(new_discontinuity);

										if (draw_cast_rays) draw_line(actual_intersection, graphics::pixel_32(0, 0, 255, 255));
									}
								}
								break;
							}
						}
					}
				}
			}

			/* now propagate the output */
			for (size_t i = 0; i < double_rays.size(); ++i) {
				/* (i + 1)%double_rays.size() ensures the cycle */
				auto& ray_a = double_rays[i];
				auto& ray_b = double_rays[(i + 1)%double_rays.size()];

				/* transform intersection locations to pixels */
				vec2<> p1 = ray_a.second * METERS_TO_PIXELSf;
				vec2<> p2 = ray_b.first * METERS_TO_PIXELSf;

				if (draw_triangle_edges) {
					draw_line(p1 * PIXELS_TO_METERSf, request.color);
					draw_line(p2 * PIXELS_TO_METERSf, request.color);
					render.lines.push_back(render_system::debug_line(p1, p2, request.color));
				}

				/* save new edge */
				request.edges.push_back(std::make_pair(p1, p2));
			}

			/* a little processing on discontinuities, we'll need them in a moment */
			for (auto& disc : request.discontinuities) {
				/* transform all discontinuities from Box2D coordinates to pixels */
				disc.points.first *= METERS_TO_PIXELSf;
				disc.points.second *= METERS_TO_PIXELSf;

				/* wrap the indices, some may be negative */
				if (disc.edge_index < 0)
					disc.edge_index = double_rays.size() - 1;
			}

			/* 
			additional processing: delete discontinuities navigation to which will result in collision
			values less than zero indicate we don't want to perform this calculation */
			if (request.ignore_discontinuities_shorter_than > 0.f) {
				int edges_num = request.edges.size();

				/* prepare helpful lambda */
				auto& wrap = [edges_num](int ix){
					if (ix < 0) return edges_num + ix;
					return ix % edges_num;
				};

				/* shortcut, note we get it by copy */
				auto discs_copy = request.discontinuities;

				/* container for edges denoting unreachable areas */
				std::vector<components::visibility::edge> marked_holes;

				/* for every discontinuity, remove if there exists any edge that is too close to the discontinuity's vertex */
				discs_copy.erase(std::remove_if(discs_copy.begin(), discs_copy.end(),
					[&request, edges_num, &transform, &wrap, &render, &marked_holes, this]
				(const components::visibility::discontinuity& d){
						std::vector<vec2<>> points_too_close;

						/* let's handle both CW and CCW cases in one go, only the sign differs somewhere */
						int cw = d.winding == d.RIGHT ? 1 : -1;
						
						/* we check all vertices of edges */
						for (int j = wrap(d.edge_index + cw), k = 0; k < edges_num-1; j = wrap(j + cw), ++k) {
							/* we've already reached edges that are to CW/CCW side of discontinuity, we're not interested in checking further */
							if (cw * (d.points.first - transform.pos).cross(request.edges[j].first - transform.pos) <= 0)
								break;

							/* project this point onto candidate edge */
							vec2<> close_point = d.points.first.closest_point_on_segment(request.edges[j].first, request.edges[j].second);

							/* if the distance is less than allowed */
							if (close_point.compare(d.points.first, request.ignore_discontinuities_shorter_than)) 
								points_too_close.push_back(close_point);
						}
						
						/* let's also check the discontinuities - we don't know what is behind them */
						for (auto& old_disc : request.discontinuities) {
							if (old_disc.edge_index != d.edge_index) {
								/* if a discontinuity is to CW/CCW respectively */
								if (!(cw * (d.points.first - transform.pos).cross(old_disc.points.first - transform.pos) <= 0)) {
									/* project this point onto candidate discontinuity */
									vec2<> close_point = d.points.first.closest_point_on_segment(old_disc.points.first, old_disc.points.second);

									if (close_point.compare(d.points.first, request.ignore_discontinuities_shorter_than))
										points_too_close.push_back(close_point);
								}
							}
						}

						/* if there is any threatening close point */
						if (!points_too_close.empty()) {
							/* pick the one closest to the entity */
							vec2<> closest_point = *std::min_element(points_too_close.begin(), points_too_close.end(), 
								[&transform](vec2<> a, vec2<> b) {
								return (a - transform.pos).length_sq() < (b - transform.pos).length_sq();
							});

							/* denote the unreachable area by saving an edge from the closest point to the discontinuity */
							marked_holes.push_back(components::visibility::edge(closest_point, d.points.first));
							
							if (draw_discontinuities)
								render.lines.push_back(render_system::debug_line(closest_point, d.points.first, graphics::pixel_32(255, 255, 255, 255)));
							
							/* remove this discontinuity */
							return true;
						}

						/* there are no threatening close points, discontinuity is ok */
						return false;
				}
				), discs_copy.end());

				/* now delete any of remaining discontinuities that can't be seen 
				through marked non-walkable areas (sort of a pathological case) 
				from position of the player
				
				prepare raycast data
				*/
				b2RayCastOutput output;
				b2RayCastInput input;
				input.maxFraction = 1.0;

				for (auto& marked : marked_holes) {
					/* prepare raycast subject */
					b2EdgeShape marked_hole;
					marked_hole.Set(marked.first, marked.second);

					/* remove every discontinuity raycast with which gives positive result */
					discs_copy.erase(std::remove_if(discs_copy.begin(), discs_copy.end(), 
						[&marked_hole, &output, &input, &transform](const components::visibility::discontinuity& d){
						input.p1 = transform.pos;
						input.p2 = d.points.first;

						/* we don't need to transform edge or ray since they are in the same space
						but we have to prepare dummy b2Transform as argument for b2EdgeShape::RayCast
						*/
						b2Transform null_transform(b2Vec2(0.f, 0.f), b2Rot(0.f));

						return (marked_hole.RayCast(&output, input, null_transform, 0));
					}), discs_copy.end());
				}

				/* save cleaned copy in actual discontinuities */
				request.discontinuities = discs_copy;
			}

			if (draw_discontinuities)
				for (auto& disc : request.discontinuities)
					render.lines.push_back(render_system::debug_line(disc.points.first, disc.points.second, graphics::pixel_32(0, 127, 255, 255)));
		}
	}
}