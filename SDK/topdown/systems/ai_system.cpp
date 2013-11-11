#include "stdafx.h"
#include "ai_system.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "physics_system.h"
#include "render_system.h"

#include "../resources/render_info.h"
#include <limits>
#include <set>

struct my_callback : public b2QueryCallback {
	std::set<b2Body*> bodies;
	entity* subject;
	b2Filter* filter;

	my_callback() : subject(nullptr), filter(nullptr) {}

	bool ReportFixture(b2Fixture* fixture) override {
		if ((b2ContactFilter::ShouldCollide(filter, &fixture->GetFilterData()))
			&&
			(entity*) fixture->GetBody()->GetUserData() != subject)
			bodies.insert(fixture->GetBody());
		return true;
	}
};

struct my_ray_callback : public b2RayCastCallback {
	entity* subject;
	b2Filter* subject_filter;

	vec2<> intersection, target;
	bool hit;

	my_ray_callback() : hit(false), subject_filter(nullptr), subject(nullptr) {}

	bool ShouldRaycast(b2Fixture* fixture) override {
		return (b2ContactFilter::ShouldCollide(subject_filter, &fixture->GetFilterData()));
	}

	float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point,
		const b2Vec2& normal, float32 fraction) override {
			intersection = point;

			hit = true;
			return fraction;
	}
};

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

ai_system::ai_system() : draw_cast_rays(false), draw_triangle_edges(true), draw_discontinuities(false), draw_memorised_walls(false) {}

int components::ai::visibility::get_num_triangles() {
	return edges.size();
}

components::ai::visibility::triangle components::ai::visibility::get_triangle(int i, augmentations::vec2<> origin) {
	components::ai::visibility::triangle tri = { origin, edges[i].first, edges[i].second };
	return tri;
}

void ai_system::process_entities(world& owner) {
	physics_system& physics = owner.get_system<physics_system>();
	render_system& render = owner.get_system<render_system>();

	for (auto it : targets) {
		/* prepare container for all the vertices that we will cast the ray to */
		static std::vector < std::pair < float, vec2< >> > all_vertices_transformed;
		all_vertices_transformed.clear();

		/* get AI data and position of the entity */
		auto& ai = it->get<components::ai>();
		auto& transform = it->get<components::transform>().current;

		/* transform entity position to Box2D coordinates */
		vec2<> position_meters = transform.pos * PIXELS_TO_METERSf;

		/* for every visibility type requested for given entity */
		for (auto& entry : ai.visibility_requests.raw) {
			/* shortcut */
			auto& request = entry.val;
			/* to Box2D coordinates */
			float vision_side_meters = request.square_side * PIXELS_TO_METERSf;

			/* prepare maximum visibility square */
			b2AABB aabb;
			aabb.lowerBound = position_meters - vision_side_meters / 2;
			aabb.upperBound = position_meters + vision_side_meters / 2;

			/* get list of all fixtures that intersect with the visibility square */
			my_callback callback;
			callback.subject = it;
			callback.filter = &request.filter;
			physics.b2world.QueryAABB(&callback, aabb);

			/* for every fixture that intersected with the visibility square */
			for (auto body : callback.bodies) {
				auto verts = reinterpret_cast<entity*>(body->GetUserData())->get<components::render>().model->get_vertices();
					/* for every vertex in given fixture's shape */
					for (auto& v : verts) {
						v *= PIXELS_TO_METERSf;

						auto position = body->GetPosition();
						/* transform angle to degrees */
						auto rotation = body->GetAngle() / 0.01745329251994329576923690768489f;

						std::pair<float, vec2<>> new_vertex;
						/* transform vertex to current entity's position and rotation */
						new_vertex.second = v.rotate(rotation, b2Vec2(0, 0)) + position;

						vec2<> diff = new_vertex.second - position_meters;
						/*
						compute angle to be compared while sorting
						source:
						http://stackoverflow.com/questions/16542042/fastest-way-to-sort-vectors-by-angle-without-actually-computing-that-angle

						save the angle in pair next to the vertex position, we will then sort the "angle-vertex" pairs by angle */
						new_vertex.first = comparable_angle(diff);

						/* save transformed vertex */
						all_vertices_transformed.push_back(new_vertex);
					}
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
			bounds[0].Set(vec2<>(whole_vision[0]) + vec2<>(-1.f, 0.f), vec2<>(whole_vision[1]) + vec2<>(1.f, 0.f));
			bounds[1].Set(vec2<>(whole_vision[1]) + vec2<>(0.f, -1.f), vec2<>(whole_vision[2]) + vec2<>(0.f, 1.f));
			bounds[2].Set(vec2<>(whole_vision[2]) + vec2<>(1.f, 0.f), vec2<>(whole_vision[3]) + vec2<>(-1.f, 0.f));
			bounds[3].Set(vec2<>(whole_vision[3]) + vec2<>(0.f, 1.f), vec2<>(whole_vision[0]) + vec2<>(0.f, -1.f));

			/* debug drawing of the visibility square */
			if (draw_cast_rays || draw_triangle_edges) {
				render.lines.push_back(render_system::debug_line((vec2<>(whole_vision[0]) + vec2<>(-1.f, 0.f))*METERS_TO_PIXELSf, (vec2<>(whole_vision[1]) + vec2<>(1.f, 0.f))*METERS_TO_PIXELSf));
				render.lines.push_back(render_system::debug_line((vec2<>(whole_vision[1]) + vec2<>(0.f, -1.f))*METERS_TO_PIXELSf, (vec2<>(whole_vision[2]) + vec2<>(0.f, 1.f))*METERS_TO_PIXELSf));
				render.lines.push_back(render_system::debug_line((vec2<>(whole_vision[2]) + vec2<>(1.f, 0.f))*METERS_TO_PIXELSf, (vec2<>(whole_vision[3]) + vec2<>(-1.f, 0.f))*METERS_TO_PIXELSf));
				render.lines.push_back(render_system::debug_line((vec2<>(whole_vision[3]) + vec2<>(0.f, 1.f))*METERS_TO_PIXELSf, (vec2<>(whole_vision[0]) + vec2<>(0.f, -1.f))*METERS_TO_PIXELSf));
			}

			/* add the visibility square to the vertices that we cast rays to, computing comparable angle in place */
			for (auto& v : whole_vision)
				all_vertices_transformed.push_back(std::make_pair(comparable_angle(v - position_meters), v));

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
			std::vector<double_ray> double_rays;

			/* debugging lambda */
			auto draw_line = [&position_meters, &render](vec2<> point, graphics::pixel_32 col){
				render.lines.push_back(render_system::debug_line(position_meters * METERS_TO_PIXELSf, point * METERS_TO_PIXELSf, col));
			};

			/* container for holding info about local discontinuities, will be used for dynamic AI navigation */
			std::vector<components::ai::visibility::discontinuity> local_discontinuities;

			for (auto& vertex : all_vertices_transformed) {
				b2Vec2* from_aabb = nullptr;

				for (auto& aabb_vert : whole_vision)
					if (vertex.second == aabb_vert)
						from_aabb = &aabb_vert;

				my_ray_callback ray_callbacks[2];

				/* save the entity pointer in ray_callback structure to not intersect with the player fixture
				it is then compared in my_ray_callback::ReportFixture
				*/
				ray_callbacks[0].subject = it;
				ray_callbacks[0].subject_filter = &request.filter;
				ray_callbacks[1] = ray_callbacks[0];
				/* create a vector in direction of vertex with length equal to the half of diagonal of the visibility square
				(majority of rays will slightly SLIGHTLY go beyond visibility square, but that's not important for now)

				ray_callbacks[0] and ray_callbacks[1] differ ONLY by an epsilon added/substracted to the angle
				*/
				ray_callbacks[0].target = position_meters + vec2<>::from_degrees((vertex.second - position_meters).get_degrees() - epsilon_ray_angle_variation) * vision_side_meters / 2 * 1.414213562373095;
				ray_callbacks[1].target = position_meters + vec2<>::from_degrees((vertex.second - position_meters).get_degrees() + epsilon_ray_angle_variation) * vision_side_meters / 2 * 1.414213562373095;

				/* cast both rays starting from the player position and ending in ray_callbacks[x].target */
				physics.b2world.RayCast(&ray_callbacks[0], position_meters, ray_callbacks[0].target);
				physics.b2world.RayCast(&ray_callbacks[1], position_meters, ray_callbacks[1].target);

				/* if we did not intersect with anything */
				if (!(ray_callbacks[0].hit || ray_callbacks[1].hit)) {
					/* we must have cast the ray against AABB */
					if (from_aabb) {
						double_rays.push_back(double_ray(*from_aabb, *from_aabb, true, true));
					}
					/* only ever happens if an object is only partially inside visibility rectangle
					may happen but ignore, handling not implemented
					*/
					else {
						bool breakpoint = true;
					}
				}
				/* both rays intersect with something */
				else if (ray_callbacks[0].hit && ray_callbacks[1].hit) {
					/* if distance between intersection and target vertex is bigger than epsilon, which means that
					BOTH intersections occured FAR from the vertex
					then ray must have intersected with an obstacle BEFORE reaching the vertex, ignoring intersection completely */
					float distance_from_origin = (vertex.second - position_meters).length_sq();
					
					if ((ray_callbacks[0].intersection - position_meters).length_sq() + PIXELS_TO_METERSf * epsilon_threshold_obstacle_hit < distance_from_origin &&
						(ray_callbacks[1].intersection - position_meters).length_sq() + PIXELS_TO_METERSf * epsilon_threshold_obstacle_hit < distance_from_origin) {
							if (draw_cast_rays) draw_line(ray_callbacks[0].intersection, graphics::pixel_32(255, 0, 0, 255));
					}
					/* distance between both intersections fit in epsilon which means ray intersected with the same vertex */
					else if ((ray_callbacks[0].intersection - ray_callbacks[1].intersection).length_sq() < PIXELS_TO_METERSf * epsilon_distance_vertex_hit) {
						/* interpret it as both rays hit the same vertex
						for maximum accuracy, push the vertex coordinates instead of the actual intersections */
						double_rays.push_back(double_ray(vertex.second, vertex.second, true, true));
						if (draw_cast_rays) draw_line(vertex.second, graphics::pixel_32(255, 255, 0, 255));
					}
					/* this is the case where the ray is cast at the lateral vertex,
					here we also detect the discontinuity */
					else {
						/* save both intersection points, this is what we introduced "double_ray" pair for */
						double_ray new_double_ray(ray_callbacks[0].intersection, ray_callbacks[1].intersection, false, false);

						/* handle new discontinuity */
						components::ai::visibility::discontinuity new_discontinuity;

						/* if the ray that we substracted the epsilon from intersected closer (and thus with the vertex), then the free space is to the right */
						if ((ray_callbacks[0].intersection - position_meters).length_sq() < (ray_callbacks[1].intersection - position_meters).length_sq()) {
							/* it was "first" one that directly reached its destination */
							new_double_ray.first_reached_destination = true;

							new_discontinuity.points.first = vertex.second;
							new_discontinuity.points.second = ray_callbacks[1].intersection;
							new_discontinuity.winding = components::ai::visibility::discontinuity::RIGHT;
							if (draw_cast_rays) draw_line(ray_callbacks[1].intersection, graphics::pixel_32(255, 0, 255, 255));
						}
						/* otherwise it is to the left */
						else {
							/* it was "second" one that directly reached its destination */
							new_double_ray.second_reached_destination = true;

							new_discontinuity.points.first = ray_callbacks[1].intersection;
							new_discontinuity.points.second = vertex.second;
							new_discontinuity.winding = components::ai::visibility::discontinuity::LEFT;
							if (draw_cast_rays) draw_line(ray_callbacks[0].intersection, graphics::pixel_32(255, 0, 255, 255));
						}
						/* save new double ray */
						double_rays.push_back(new_double_ray);
						/* save new discontinuity */
						local_discontinuities.push_back(new_discontinuity);
					}
				}
				/* the case where exactly one of the rays did not hit anything so we cast it against boundaries,
				we also detect discontinuity here */
				else {
					/* for every callback that didn't detect hit (there will be only one) */
					for (int i = 0; i < 2; ++i) {
						if (!ray_callbacks[i].hit) {
							/* for every edge from 4 edges forming visibility square */
							for (auto& bound : bounds) {
								/* prepare b2RayCastOutput/b2RayCastInput data for raw b2EdgeShape::RayCast call */
								b2RayCastOutput output;
								b2RayCastInput input;
								input.maxFraction = 1.0;
								input.p1 = position_meters;
								input.p2 = ray_callbacks[i].target;

								/* we don't need to transform edge or ray since they are in the same space
								but we have to prepare dummy b2Transform as argument for b2EdgeShape::RayCast
								*/
								b2Transform null_transform(b2Vec2(0.f, 0.f), b2Rot(0.f));

								/* if we hit against boundaries (must happen for at least 1 of them) */
								if (bound.RayCast(&output, input, null_transform, 0)) {
									/* prepare new discontinuity data */
									components::ai::visibility::discontinuity new_discontinuity;

									/* compute the actual intersection point from b2RayCastOutput data */
									auto actual_intersection = input.p1 + output.fraction * (input.p2 - input.p1);

									/* if the left-handed ray intersected with boundary */
									if (i == 0) {
										new_discontinuity.points.first = ray_callbacks[1].intersection;
										new_discontinuity.points.second = ray_callbacks[0].intersection;
										new_discontinuity.winding = components::ai::visibility::discontinuity::LEFT;
										double_rays.push_back(double_ray(actual_intersection, ray_callbacks[1].intersection, false, true));
									}
									/* if the right-handed ray intersected with boundary */
									else if (i == 1) {
										new_discontinuity.points.first = ray_callbacks[0].intersection;
										new_discontinuity.points.second = ray_callbacks[1].intersection;
										new_discontinuity.winding = components::ai::visibility::discontinuity::RIGHT;
										double_rays.push_back(double_ray(ray_callbacks[0].intersection, actual_intersection, true, false));
									}

									local_discontinuities.push_back(new_discontinuity);

									if (draw_cast_rays) draw_line(actual_intersection, graphics::pixel_32(0, 0, 255, 255));
								}
							}
							break;
						}
					}
				}
			}

			/* now to propagate the visible area */
			request.edges.clear();

			if (draw_discontinuities)
				for (auto& disc : local_discontinuities)
					render.lines.push_back(render_system::debug_line(disc.points.first, disc.points.second, graphics::pixel_32(0, 127, 255, 255)));

			for (size_t i = 0; i < double_rays.size(); ++i) {
				/* (i + 1)%double_rays.size() ensures the cycle */
				auto& ray_a = double_rays[i];
				auto& ray_b = double_rays[(i + 1)%double_rays.size()];

				vec2<> p1 = ray_a.second * METERS_TO_PIXELSf;
				vec2<> p2 = ray_b.first * METERS_TO_PIXELSf;

				/* we have a fully visible wall here */
				if (ray_a.second_reached_destination && ray_b.first_reached_destination) {
					/* compare with already memorised walls */
					bool such_a_wall_exists = false;
					for (auto& wall : request.memorised_walls) {
						/* if any memorised wall is almost equal to the candidate */
						if (((wall.first - p1).length_sq() < epsilon_max_segment_difference && (wall.second - p2).length_sq() < epsilon_max_segment_difference)
							||
							((wall.first - p2).length_sq() < epsilon_max_segment_difference && (wall.second - p1).length_sq() < epsilon_max_segment_difference)
							) {
								/* there's no need to push back the new one*/
								such_a_wall_exists = true;
								break;
						}
					}

					if (!such_a_wall_exists)  {
						request.memorised_walls.push_back(components::ai::edge(p1, p2));

						/* if such exists, delete the undiscovered wall that falls into this discovered wall */
						auto& walls = request.memorised_undiscovered_walls;
						//walls.erase(std::remove_if(walls.begin(), walls.end(), [&](const components::ai::edge& e){
						//	return vec2<>::segment_in_segment(e.first, e.second, p1, p2, epsilon_max_segment_difference);
						//}), walls.end());
						
						for (auto& it = walls.begin(); it != walls.end(); ++it) {
							if (vec2<>::segment_in_segment((*it).first, (*it).second, p1, p2, epsilon_max_segment_difference)) {
								walls.erase(it);
								break;
							}
						}
					}
				}
				/* we have a potentially undiscovered wall */
				else {
					bool this_wall_is_discovered = false;
					/* if any memorised, discovered wall covers the the candidate */
					for (auto& wall : request.memorised_walls) {
						if (vec2<>::segment_in_segment(p1, p2, wall.first, wall.second, epsilon_max_segment_difference)) {
							/* we don't push it as "undiscovered" as it is already discovered */
							this_wall_is_discovered = true;
							break;
						}
					}

					/* if this wall is undiscovered */
					if (!this_wall_is_discovered) {
						bool this_wall_covers_any_undiscovered_wall = false;

						for (auto& wall : request.memorised_undiscovered_walls) {
							/* check if our undiscovered candidate fits into
							any existing undiscovered walls */
							if (vec2<>::segment_in_segment(p1, p2, wall.first, wall.second, epsilon_max_segment_difference)) {
								/* if it is so, don't do anything and break */
								this_wall_covers_any_undiscovered_wall = true;
								break;
							}
							/* reverse roles: we check if any existing undiscovered wall fits inside
							our undiscovered candidate */
							if (vec2<>::segment_in_segment(wall.first, wall.second, p1, p2, epsilon_max_segment_difference)) {
								/* we don't push (p1, p2) as another undiscovered wall,
								we just widen this existing one */
								this_wall_covers_any_undiscovered_wall = true;

								wall.first = p1;
								wall.second = p2;
								break;
							}
						}

						/* if there's no such wall that can replace the new one */
						if (!this_wall_covers_any_undiscovered_wall) {
							request.memorised_undiscovered_walls.push_back(components::ai::edge(p1, p2));
						}
					}
				}

				if (draw_triangle_edges) {
					draw_line(p1 * PIXELS_TO_METERSf, request.color);
					draw_line(p2 * PIXELS_TO_METERSf, request.color);
					render.lines.push_back(render_system::debug_line(p1, p2, request.color));
				}

				request.edges.push_back(std::make_pair(p1, p2));
			}

			if (draw_memorised_walls) {
				for (auto& wall : request.memorised_walls) {
					render.lines.push_back(render_system::debug_line(wall.first, wall.second, graphics::pixel_32(0, 255, 0, 255)));
				}

				for (auto& wall : request.memorised_undiscovered_walls) {
					//render.lines.push_back(render_system::debug_line(wall.first, wall.second, graphics::pixel_32(255, 0, 0, 255)));
				}
			}
		}
	}
}