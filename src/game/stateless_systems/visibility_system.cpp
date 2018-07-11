#include <limits>

#include <Box2D/Box2D.h>

#include "augs/math/math.h"
#include "augs/templates/container_templates.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/misc/simple_pair.h"
#include "game/detail/physics/physics_queries.h"
#include "game/debug_drawing_settings.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/stateless_systems/visibility_system.h"
#include "game/inferred_caches/physics_world_cache.h"

#include "game/components/rigid_body_component.h"
#include "game/components/transform_component.h"
#include "game/components/item_component.h"
#include "game/components/sentience_component.h"
#include "game/components/attitude_component.h"

#include "game/messages/visibility_information.h"

#include "game/detail/entity_scripts.h"
#include "game/detail/physics/physics_scripts.h"

using namespace augs;
using namespace messages;

/*
source:
http://stackoverflow.com/questions/16542042/fastest-way-to-sort-vectors-by-angle-without-actually-computing-that-angle
*/
static auto comparable_angle(const vec2 diff) {
	return augs::sgn(diff.y) * (
		1 - (diff.x / (std::abs(diff.x) + std::abs(diff.y)))
	);
}

using edge = visibility_information_response::edge;
using discontinuity = visibility_information_response::discontinuity;
using triangle = visibility_information_response::triangle;

size_t visibility_information_response::get_num_triangles() const {
	return edges.size();
}

discontinuity* visibility_information_response::get_discontinuity_for_edge(const size_t n) {
	for (auto& disc : discontinuities) {
		if (disc.edge_index == static_cast<int>(n)) {
			return &disc;
		}
	}

	return nullptr;
}

discontinuity* visibility_information_response::get_discontinuity(const size_t n) {
	return &discontinuities[n];
}

const discontinuity* visibility_information_response::get_discontinuity_for_edge(const size_t n) const {
	for (auto& disc : discontinuities) {
		if (disc.edge_index == static_cast<int>(n)) {
			return &disc;
		}
	}

	return nullptr;
}

const discontinuity* visibility_information_response::get_discontinuity(const size_t n) const {
	return &discontinuities[n];
}

triangle visibility_information_response::get_world_triangle(const size_t i, const vec2 origin) const {
	return { origin, edges[i].first, edges[i].second };
}

std::vector<vec2> visibility_information_response::get_world_polygon(
	const real32 distance_epsilon, 
	const vec2 expand_origin, 
	const real32 expand_mult
) const {
	std::vector<vec2> output;

	for (size_t i = 0; i < edges.size(); ++i) {
		if (
			std::isnan(edges[i].first.x) ||
			std::isnan(edges[i].first.y) ||
			std::isnan(edges[i].second.x) ||
			std::isnan(edges[i].second.y)
			)
		{
			break;
		}

		/* always push the first point */
		output.push_back(edges[i].first);

		/* push the second one only if it is different from the first point of the next edge */
		if (!edges[i].second.compare(edges[(i + 1) % edges.size()].first, distance_epsilon)) {
			output.push_back(edges[i].second + (edges[i].second - expand_origin)*expand_mult);
		}
	}

	return output;
}

bool line_of_sight_response::sees(const entity_id id) const {
	return visible_items.find(id) != visible_items.end()
		|| visible_sentiences.find(id) != visible_sentiences.end()
		|| visible_attitudes.find(id) != visible_attitudes.end()
		|| visible_dangers.find(id) != visible_dangers.end()
		;
}

void visibility_system::respond_to_visibility_information_requests(const logic_step step) const {
	const auto& los_requests = step.get_queue<messages::line_of_sight_request>();
	const auto& vis_requests = step.get_queue<messages::visibility_information_request>();
	auto& los_responses = step.get_queue<messages::line_of_sight_response>();
	auto& vis_responses = step.get_queue<messages::visibility_information_response>();

	respond_to_visibility_information_requests(step.get_cosmos(),
		los_requests,
		vis_requests,
		los_responses,
		vis_responses
	);

	for (size_t i = 0; i < los_requests.size(); ++i) {
		step.transient.calculated_line_of_sight[los_requests[i].subject] = los_responses[i];
	}

	for (size_t i = 0; i < vis_requests.size(); ++i) {
		step.transient.calculated_visibility[vis_requests[i].subject] = vis_responses[i];
	}
}

visibility_system::visibility_responses visibility_system::respond_to_visibility_information_requests(
	const cosmos& cosmos,
	const std::vector<messages::line_of_sight_request>& los_requests,
	const std::vector<messages::visibility_information_request>& vis_requests
) {
	visibility_system::visibility_responses output;

	respond_to_visibility_information_requests(
		cosmos,
		los_requests,
		vis_requests,
		output.los,
		output.vis
	);
	
	return output;
}

void visibility_system::respond_to_visibility_information_requests(
	const cosmos& cosmos,
	const std::vector<messages::line_of_sight_request>& los_requests,
	const std::vector<messages::visibility_information_request>& vis_requests,
	std::vector<messages::line_of_sight_response>& los_responses,
	std::vector<messages::visibility_information_response>& vis_responses
) const {
	const auto si = cosmos.get_si();

	const auto settings = [&cosmos](){ 
		auto absolutize = [](float& f) {
			f = std::fabs(f);
		};

		auto s = cosmos.get_common_significant().visibility;

		absolutize(s.epsilon_distance_vertex_hit);
		absolutize(s.epsilon_ray_distance_variation);
		absolutize(s.epsilon_threshold_obstacle_hit);

		return s;
	}();


	auto& lines = DEBUG_LINES_TARGET;

	/* prepare epsilons to be used later, just to make the notation more clear */
	const auto epsilon_distance_vertex_hit_sq =
		si.get_meters(settings.epsilon_distance_vertex_hit) *
		si.get_meters(settings.epsilon_distance_vertex_hit)
	;

	const auto epsilon_threshold_obstacle_hit_meters = si.get_meters(settings.epsilon_threshold_obstacle_hit);

	/* we'll need a reference to physics system for raycasting */
	const physics_world_cache& physics = cosmos.get_solvable_inferred().physics;

	struct ray_input {
		vec2 targets[2];
	};

	std::vector<
		augs::simple_pair<
			physics_raycast_output, 
			physics_raycast_output
		>
	> all_ray_outputs;
	std::vector<ray_input> all_ray_inputs;

	for (const auto& request : los_requests) {
		const auto it = cosmos[request.subject];
		const auto transform = it.get_logic_transform();

		line_of_sight_response response;

		const auto d = request.maximum_distance;
		
		physics.for_each_in_aabb(
			si,
			transform.pos - vec2(d, d), 
			transform.pos + vec2(d, d), 
			request.candidate_filter, 
			[&](const b2Fixture* const fix) {
				const auto candidate = cosmos[get_entity_that_owns(fix)];
				
				if (candidate.get_owner_of_colliders() == it) {
					return callback_result::CONTINUE;
				}

				const auto target_pos = candidate.get_logic_transform().pos;

				if ((target_pos - transform.pos).length_sq() <= d*d) {
					thread_local std::vector<std::unordered_set<entity_id>*> target_sets;
					target_sets.clear();

					if (request.test_items) {
						if (candidate.has<components::item>()) {
							target_sets.push_back(&response.visible_items);
						}
					}

					if (request.test_sentiences) {
						if (candidate.has<components::sentience>()) {
							target_sets.push_back(&response.visible_sentiences);
						}
					}

					if (request.test_attitudes) {
						if (candidate.has<components::attitude>()) {
							target_sets.push_back(&response.visible_attitudes);
						}
					}

					if (request.test_dangers) {
						if (assess_danger(it, candidate).amount > 0) {
							target_sets.push_back(&response.visible_dangers);
						}
					}

					if (target_sets.size() > 0) {
						const auto out = physics.ray_cast_px(
							cosmos.get_si(), 
							transform.pos, 
							target_pos, 
							request.obstruction_filter, 
							it
						);

						const auto line_of_sight_unobstructed = !out.hit;

						if (line_of_sight_unobstructed) {
							for (const auto& t : target_sets) {
								t->insert(candidate);
							}
						}
					}
				}

				return callback_result::CONTINUE;
			}
		);

		los_responses.emplace_back(std::move(response));
	}

	for (const auto& request : vis_requests) {
		const auto ignored_entity = request.subject;
		const auto transform = request.eye_transform;
		visibility_information_response response;

		response.source_square_side = request.square_side;

		/* prepare container for all the vertices that we will cast the ray to */
		struct target_vertex {
			bool is_on_a_bound;
			real32 angle;
			vec2 pos;

			bool operator<(const target_vertex& b) const {
				return angle < b.angle;
			}

			bool operator==(const target_vertex& b) const {
				return pos.compare(b.pos) || (b.angle - angle) <= std::numeric_limits<decltype(angle)>::epsilon();
			}
		};

		thread_local std::vector <target_vertex> all_vertices_transformed;
		all_vertices_transformed.clear();

		/* transform entity position to Box2D coordinates and take offset into account */
		const vec2 position_meters = si.get_meters(transform.pos + request.offset);

		/* to Box2D coordinates */
		const auto vision_side_meters = si.get_meters(request.square_side);

		/* prepare maximum visibility square */
		b2AABB aabb;
		aabb.lowerBound = b2Vec2(position_meters - vision_side_meters / 2);
		aabb.upperBound = b2Vec2(position_meters + vision_side_meters / 2);

		ltrb ltrb(aabb.lowerBound.x, aabb.lowerBound.y, aabb.upperBound.x, aabb.upperBound.y);

		const auto push_vertex = [position_meters, ltrb](
			const vec2 v, 
			const bool check_against_aabb
		) {
			/* don't bother if it does not hover the aabb */
			if (check_against_aabb && !ltrb.hover(vec2(v))) {
				return;
			}

			target_vertex new_vertex;
			new_vertex.pos = v;

			/* calculate difference vector */
			vec2 diff = new_vertex.pos - position_meters;
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

		/* for every fixture that intersected with the visibility square */
		physics.for_each_in_aabb_meters(
			aabb, 
			request.filter,
			[&](const b2Fixture* const f) {
				if (get_body_entity_that_owns(f) == Userdata(ignored_entity)) {
					return callback_result::CONTINUE;
				}

				const auto* const shape = f->m_shape;
				/* get shape vertices from misc that transforms them to current entity's position and rotation in Box2D space */
				if (shape->GetType() == b2Shape::e_polygon) {
					const auto* const poly = static_cast<const b2PolygonShape*>(shape);

					for (int32 vp = 0; vp < poly->GetVertexCount(); ++vp) {
						const auto vv = poly->GetVertex(vp);
						const auto position = f->GetBody()->GetPosition();
						const auto rotation = f->GetBody()->GetAngle();

						/* transform vertex to current entity's position and rotation */
						push_vertex(vec2(vv).rotate_radians(rotation) + vec2(position), true);
					}
				}

				return callback_result::CONTINUE;
			}
		);

		/* extract the actual vertices from visibility AABB to cast rays to */
		const b2Vec2 whole_vision[] = {
			aabb.lowerBound,
			aabb.lowerBound + b2Vec2(vision_side_meters, 0),
			aabb.upperBound,
			aabb.upperBound - b2Vec2(vision_side_meters, 0)
		};

		/* prepare edge shapes given above vertices to cast rays against when no obstacle was hit
		note we lengthen them a bit and add/substract 1.f to avoid undeterministic vertex cases
		*/
		b2EdgeShape bounds[4];
		const auto moving_epsilon = si.get_meters(1.f);
		bounds[0].Set(b2Vec2(whole_vision[0]) + b2Vec2(-moving_epsilon, 0.f), b2Vec2(whole_vision[1]) + b2Vec2(moving_epsilon, 0.f));
		bounds[1].Set(b2Vec2(whole_vision[1]) + b2Vec2(0.f, -moving_epsilon), b2Vec2(whole_vision[2]) + b2Vec2(0.f, moving_epsilon));
		bounds[2].Set(b2Vec2(whole_vision[2]) + b2Vec2(moving_epsilon, 0.f), b2Vec2(whole_vision[3]) + b2Vec2(-moving_epsilon, 0.f));
		bounds[3].Set(b2Vec2(whole_vision[3]) + b2Vec2(0.f, moving_epsilon), b2Vec2(whole_vision[0]) + b2Vec2(0.f, -moving_epsilon));

		/* debug drawing of the visibility square */
		if (DEBUG_DRAWING.draw_cast_rays || DEBUG_DRAWING.draw_triangle_edges) {
			lines.emplace_back(white, si.get_pixels(vec2(whole_vision[0]) + vec2(-moving_epsilon, 0.f)), si.get_pixels(vec2(whole_vision[1]) + vec2(moving_epsilon, 0.f)));
			lines.emplace_back(white, si.get_pixels(vec2(whole_vision[1]) + vec2(0.f, -moving_epsilon)), si.get_pixels(vec2(whole_vision[2]) + vec2(0.f, moving_epsilon)));
			lines.emplace_back(white, si.get_pixels(vec2(whole_vision[2]) + vec2(moving_epsilon, 0.f)),  si.get_pixels(vec2(whole_vision[3]) + vec2(-moving_epsilon, 0.f)));
			lines.emplace_back(white, si.get_pixels(vec2(whole_vision[3]) + vec2(0.f, moving_epsilon)),  si.get_pixels(vec2(whole_vision[0]) + vec2(0.f, -moving_epsilon)));
		}

		/* raycast through the bounds to add another vertices where the shapes go beyond visibility square */
		for (const auto& bound : bounds) {
			/* have to raycast both directions because Box2D ignores the second side of the fixture */
			const auto output1 = physics.ray_cast_all_intersections(bound.m_vertex1, bound.m_vertex2, request.filter, ignored_entity);
			const auto output2 = physics.ray_cast_all_intersections(bound.m_vertex2, bound.m_vertex1, request.filter, ignored_entity);

			/* check for duplicates */
			std::vector<vec2> output;

			for (const auto& inter : output1) {
				output.push_back(inter.intersection);
			}

			for (const auto& v : output2) {
				bool duplicate_found = false;

				for (auto& duplicate : output1) {
					if (v.intersection.compare(duplicate.intersection)) {
						duplicate_found = true;
						break;
					}
				}

				if (!duplicate_found) {
					output.push_back(v.intersection);
				}
			}

			if (DEBUG_DRAWING.draw_cast_rays) {
				lines.emplace_back(si.get_pixels(bound.m_vertex1), si.get_pixels(bound.m_vertex2), rgba(255, 0, 0, 255));
			}

			for (const auto v : output) {
				push_vertex(v, false);
			}
		}

		/* add the visibility square to the vertices that we cast rays to, computing comparable angle in place */
		for (const auto v : whole_vision) {
			push_vertex(v, false);
		}

		/* SORT ALL VERTICES BY ANGLE */
		sort_range(all_vertices_transformed);
		remove_duplicates_from_sorted(all_vertices_transformed);

		/* by now we have ensured that all_vertices_transformed is non-empty

		debugging:
		red ray - ray that intersected with obstacle, these are ignored
		yellow ray - ray that hit the same vertex
		violet ray - ray that passed through vertex and hit another obstacle
		blue ray - ray that passed through vertex and hit boundary
		*/

		/* double_ray pair for holding both left-epsilon and right-epsilon rays */
		struct double_ray {
			vec2 first;
			vec2 second;
			bool first_reached_destination = false;
			bool second_reached_destination = false;

			double_ray() = default;

			double_ray(
				const vec2 first,
				const vec2 second, 
				const bool a, 
				const bool b
			) : 
				first(first), 
				second(second), 
				first_reached_destination(a), 
				second_reached_destination(b) 
			{
			}
		};

		/* container for these */
		std::vector<double_ray> double_rays;

		const auto push_double_ray = [&](const double_ray& ray_b) -> bool {
			bool is_same_as_previous = false;
			const vec2 p2 = si.get_pixels(ray_b.first);

			if (!double_rays.empty()) {
				const auto& ray_a = *double_rays.rbegin();
				const vec2 p1 = si.get_pixels(ray_a.second);

				is_same_as_previous = p1.compare(p2);
			}

			/* save new double_ray if it is not degenerate */
			if (
				!is_same_as_previous
				&& (std::fpclassify(p2.x) == FP_NORMAL || std::fpclassify(p2.x) == FP_ZERO)
				&& (std::fpclassify(p2.y) == FP_NORMAL || std::fpclassify(p2.y) == FP_ZERO)
				&& p2.x == p2.x
				&& p2.y == p2.y
			) {
				double_rays.push_back(ray_b);
				return true;
			}

			return false;
		};


		/* helper debugging lambda */
		const auto draw_line = [&](const vec2 point, const rgba col) {
			lines.emplace_back(si.get_pixels(position_meters), si.get_pixels(point), col);
		};

		all_ray_inputs.clear();
		all_ray_outputs.clear();

		all_ray_inputs.reserve(all_vertices_transformed.size());
		all_ray_outputs.reserve(all_vertices_transformed.size());

		/* for every vertex to cast the ray to */
		for (const auto& vertex : all_vertices_transformed) {
			/* create two vectors in direction of vertex with length equal to the half of diagonal of the visibility square
			(majority of rays will SLIGHTLY go beyond visibility square, but that's not important now)
			ray_callbacks[0] and ray_callbacks[1] differ ONLY by an epsilon added / substracted to the angle
			*/

			/* calculate the perpendicular direction to properly apply epsilon_ray_distance_variation */
			const vec2 perpendicular_cw = (vertex.pos - position_meters).normalize().perpendicular_cw();

			const vec2 directions[2] = {
				((vertex.pos - perpendicular_cw * si.get_meters(settings.epsilon_ray_distance_variation)) - position_meters).normalize(),
				((vertex.pos + perpendicular_cw * si.get_meters(settings.epsilon_ray_distance_variation)) - position_meters).normalize()
			};

			vec2 targets[2] = {
				position_meters + directions[0] * vision_side_meters / 2 * 1.5f,
				position_meters + directions[1] * vision_side_meters / 2 * 1.5f
			};

			/* clamp the ray to the bound */
			for (const auto& bound : bounds) {
				bool continue_checking = true;

				for (int j = 0; j < 2; ++j) {
					const auto edge_ray_output = segment_segment_intersection(
						position_meters, 
						targets[j], 
						bound.m_vertex1, 
						bound.m_vertex2
					);

					if (edge_ray_output.hit) {
						/* move the target further by epsilon */
						targets[j] = edge_ray_output.intersection + directions[j] * si.get_meters(1.f);
						continue_checking = false;
					}
				/*
					else {
						bool breakpoint = true;
					}
				*/
				}

				if (!continue_checking) {
					break;
				}
			}

			/* cast both rays starting from the player position and ending in targets[x].target,
			ignoring subject entity ("it") completely, save results in ray_callbacks[2] */
			ray_input new_ray_input;

			new_ray_input.targets[0] = vertex.is_on_a_bound ? vertex.pos : targets[0];
			new_ray_input.targets[1] = targets[1];
			all_ray_inputs.push_back(new_ray_input);
		}

		/* process all raycast inputs at once to improve cache coherency */
		for (size_t j = 0; j < all_ray_inputs.size(); ++j) {
			all_ray_outputs.emplace_back(
				physics.ray_cast(position_meters, all_ray_inputs[j].targets[0], request.filter, ignored_entity),
				all_vertices_transformed[j].is_on_a_bound ?
				physics_raycast_output() : physics.ray_cast(position_meters, all_ray_inputs[j].targets[1], request.filter, ignored_entity)
			);
		}

		for (size_t i = 0; i < all_ray_outputs.size(); ++i) {
			physics_raycast_output ray_callbacks[2] = { all_ray_outputs[i].first, all_ray_outputs[i].second };
			auto& vertex = all_vertices_transformed[i];

			/* const b2Vec2* from_aabb = nullptr; */

			/* if the vertex comes from bounding square, save it and remember about it */
			/* for (const auto& aabb_vert : whole_vision) { */
			/* 	if (vertex.pos == aabb_vert) { */
			/* 		from_aabb = &aabb_vert; */
			/* 	} */
			/* } */

			if (vertex.is_on_a_bound && (!ray_callbacks[0].hit || (ray_callbacks[0].intersection - vertex.pos).length_sq() < epsilon_distance_vertex_hit_sq)) {
				/* if it is a vertex on the boundary, handle it accordingly - interpret it as a new discontinuity (e.g. for pathfinding) */
				discontinuity new_discontinuity;
				new_discontinuity.edge_index = static_cast<int>(double_rays.size());
				new_discontinuity.points.first = vertex.pos;

				vec2 actual_normal;// = (ray_callbacks[0].normal / 2;
				vec2 actual_intersection;// = (+ray_callbacks[1].intersection) / 2;
				new_discontinuity.points.second = ray_callbacks[0].intersection;
				new_discontinuity.normal = actual_normal;

				new_discontinuity.winding = actual_normal.cross(position_meters - actual_intersection) > 0 ?
					discontinuity::RIGHT : discontinuity::LEFT;

				/* if it is clockwise, we take previous edge as subject */
				if (new_discontinuity.winding == discontinuity::RIGHT) {
					--new_discontinuity.edge_index;
				}

				new_discontinuity.is_boundary = true;
				//request.discontinuities.push_back(new_discontinuity);
				if (push_double_ray(double_ray(vertex.pos, vertex.pos, true, true)))
					if (DEBUG_DRAWING.draw_cast_rays) draw_line(vertex.pos, rgba(255, 255, 0, 255));
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
					const auto distance_from_origin = (vertex.pos - position_meters).length();

					if ((ray_callbacks[0].intersection - position_meters).length() + epsilon_threshold_obstacle_hit_meters < distance_from_origin &&
						(ray_callbacks[1].intersection - position_meters).length() + epsilon_threshold_obstacle_hit_meters < distance_from_origin) {
						if (DEBUG_DRAWING.draw_cast_rays) draw_line(vertex.pos, rgba(255, 0, 0, 255));
					}
					/* distance between both intersections fit in epsilon which means ray intersected with the same vertex */
					else if ((ray_callbacks[0].intersection - ray_callbacks[1].intersection).length_sq() < epsilon_distance_vertex_hit_sq) {
						/* interpret it as both rays hit the same vertex
						for maximum accuracy, push the vertex coordinates instead of the actual intersections */

						if (push_double_ray(double_ray(vertex.pos, vertex.pos, true, true))) {
							response.vertex_hits.emplace_back(
								static_cast<int>(double_rays.size()) - 1, 
								si.get_pixels(vertex.pos)
							);

							if (DEBUG_DRAWING.draw_cast_rays) draw_line(vertex.pos, rgba(255, 255, 0, 255));
						}
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
						discontinuity new_discontinuity;

						/* if the ray that we substracted the epsilon from intersected closer (and thus with the vertex), then the free space is to the right */
						if ((ray_callbacks[0].intersection - vertex.pos).length_sq() <
							(ray_callbacks[1].intersection - vertex.pos).length_sq()) {
							/* it was "first" one that directly reached its destination */
							new_double_ray.first_reached_destination = true;
							new_double_ray.first = vertex.pos;

							/* save discontinuity info and edge_index to associate discontinuity with current edge */
							new_discontinuity.points.first = vertex.pos;
							new_discontinuity.points.second = ray_callbacks[1].intersection;
							new_discontinuity.winding = discontinuity::RIGHT;
							new_discontinuity.edge_index = static_cast<int>(double_rays.size() - 1);
							if (DEBUG_DRAWING.draw_cast_rays) draw_line(ray_callbacks[1].intersection, rgba(255, 0, 255, 255));
						}
						/* otherwise the free area is to the left */
						else {
							/* it was "second" one that directly reached its destination */
							new_double_ray.second_reached_destination = true;
							new_double_ray.second = vertex.pos;

							new_discontinuity.points.first = vertex.pos;
							new_discontinuity.points.second = ray_callbacks[0].intersection;
							new_discontinuity.winding = discontinuity::LEFT;
							new_discontinuity.edge_index = static_cast<int>(double_rays.size());
							if (DEBUG_DRAWING.draw_cast_rays) draw_line(ray_callbacks[0].intersection, rgba(255, 0, 255, 255));
						}

						/* save new double ray */
						if (push_double_ray(new_double_ray)) {
							/* save new discontinuity */
							response.discontinuities.push_back(new_discontinuity);
						}
					}
				}
				/* the case where exactly one of the rays did not hit anything so we cast it against boundaries,
				we also detect discontinuity here */
				else {
					/* for every callback that didn't detect hit (there will be only one) */
					for (size_t k = 0; k < 2; ++k) {
						if (!ray_callbacks[k].hit) {
							/* for every edge from 4 edges forming visibility square */
							for (const auto& bound : bounds) {
								const auto ray_edge_output = segment_segment_intersection(
									position_meters, 
									all_ray_inputs[i].targets[k], 
									bound.m_vertex1, 
									bound.m_vertex2
								);

								/* if we hit against boundaries (must happen for at least 1 of them) */
								if (ray_edge_output.hit) {
									/* prepare new discontinuity data */
									discontinuity new_discontinuity;

									/* compute the actual intersection point from b2RayCastOutput data */
									auto actual_intersection = ray_edge_output.intersection;

									new_discontinuity.points.first = vertex.pos;
									new_discontinuity.points.second = actual_intersection;

									double_ray new_double_ray;
									/* if the left-handed ray intersected with boundary and thus the right-handed intersected with an obstacle */
									if (k == 0) {
										new_discontinuity.winding = discontinuity::LEFT;
										new_discontinuity.edge_index = static_cast<int>(double_rays.size());
										new_double_ray = double_ray(actual_intersection, vertex.pos, false, true);
									}
									/* if the right-handed ray intersected with boundary and thus the left-handed intersected with an obstacle */
									else if (k == 1) {
										new_discontinuity.winding = discontinuity::RIGHT;
										new_discontinuity.edge_index = static_cast<int>(double_rays.size()) - 1;
										new_double_ray = double_ray(vertex.pos, actual_intersection, true, false);
									}

									/* save new double ray */
									if (push_double_ray(new_double_ray)) {
										response.discontinuities.push_back(new_discontinuity);
										if (DEBUG_DRAWING.draw_cast_rays) draw_line(actual_intersection, rgba(0, 0, 255, 255));
									}
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
			const auto& ray_a = double_rays[i];
			const auto& ray_b = double_rays[(i + 1) % double_rays.size()];

			/* transform intersection locations to pixels */
			vec2 p1 = si.get_pixels(ray_a.second);
			vec2 p2 = si.get_pixels(ray_b.first);

			//if (DEBUG_DRAWING.draw_triangle_edges) {
			//	draw_line(si.get_pixels(p1), request.color);
			//	draw_line(si.get_pixels(p2), request.color);
			//	lines.emplace_back(p1, p2, request.color);
			//}

			response.edges.emplace_back(p1, p2);
		}

		/* a little processing on discontinuities, we'll need them in a moment */
		for (auto& disc : response.discontinuities) {
			/* transform all discontinuities from Box2D coordinates to pixels */
			disc.points.first = si.get_pixels(disc.points.first);
			disc.points.second = si.get_pixels(disc.points.second);

			/* wrap the indices, some may be negative */
			if (disc.edge_index < 0) {
				disc.edge_index = static_cast<int>(double_rays.size()) - 1;
			}
		}

		/*
		additional processing: delete discontinuities navigation to which will result in collision
		values less than zero indicate we don't want to perform this calculation */
		if (request.ignore_discontinuities_shorter_than > 0.f) {
			const auto edges_num = static_cast<int>(response.edges.size());

			/* prepare helpful lambda */
			auto wrap = [edges_num](const int ix) {
				if (ix < 0) return edges_num + ix;
				return ix % edges_num;
			};

			/* shortcut, note we get it by copy */
			auto discs_copy = response.discontinuities;

			/* container for edges denoting unreachable areas */
			std::vector<edge> marked_holes;

			/* for every discontinuity, remove if there exists any edge that is too close to the discontinuity's vertex */
			discs_copy.erase(std::remove_if(discs_copy.begin(), discs_copy.end(),
				[&request, edges_num, &transform, &wrap, &lines, &marked_holes, &response]
			(const discontinuity& d) {
				std::vector<vec2> points_too_close;

				/* let's handle both CW and CCW cases in one go, only the sign differs somewhere */
				const int cw = d.winding == d.RIGHT ? 1 : -1;

				/* we check all vertices of edges */
				for (int j = wrap(static_cast<int>(d.edge_index) + cw), k = 0; k < edges_num - 1; j = wrap(j + cw), ++k) {
					/* if any of the two points of the edge is to the CW/CCW side of discontinuity */
					if (cw * (d.points.first - transform.pos).cross(response.edges[j].first - transform.pos) >= 0
						||
						cw * (d.points.first - transform.pos).cross(response.edges[j].second - transform.pos) >= 0
						) {
						/* project this point onto candidate edge */
						vec2 close_point = d.points.first.closest_point_on_segment(response.edges[j].first, response.edges[j].second);

						/* if the distance is less than allowed */
						if (close_point.compare(d.points.first, request.ignore_discontinuities_shorter_than))
							points_too_close.push_back(close_point);
					}
				}

				/* let's also check the discontinuities - we don't know what is behind them */
				for (const auto& old_disc : response.discontinuities) {
					if (old_disc.edge_index != d.edge_index) {
						/* if a discontinuity is to CW/CCW respectively */
						if (!(cw * (d.points.first - transform.pos).cross(old_disc.points.first - transform.pos) <= 0)) {
							/* project this point onto candidate discontinuity */
							vec2 close_point = d.points.first.closest_point_on_segment(old_disc.points.first, old_disc.points.second);

							if (close_point.compare(d.points.first, request.ignore_discontinuities_shorter_than))
								points_too_close.push_back(close_point);
						}
					}
				}

				/* if there is any threatening close point */
				if (!points_too_close.empty()) {
					/* pick the one closest to the entity */
					const vec2 closest_point = *std::min_element(points_too_close.begin(), points_too_close.end(),
						[&transform](vec2 a, vec2 b) {
						return (a - transform.pos).length_sq() < (b - transform.pos).length_sq();
					});

					/* denote the unreachable area by saving an edge from the closest point to the discontinuity */
					marked_holes.push_back(edge(closest_point, d.points.first));

					if (DEBUG_DRAWING.draw_discontinuities)
						lines.emplace_back(white, closest_point, d.points.first);

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

			for (const auto& marked : marked_holes) {
				/* prepare raycast subject */
				b2EdgeShape marked_hole;
				marked_hole.Set(b2Vec2(marked.first), b2Vec2(marked.second));

				/* remove every discontinuity raycast with which gives positive result */
				discs_copy.erase(std::remove_if(discs_copy.begin(), discs_copy.end(),
					[&marked_hole, &output, &input, &transform](const discontinuity& d) {
					input.p1 = b2Vec2(transform.pos);
					input.p2 = b2Vec2(d.points.first);

					/* we don't need to transform edge or ray since they are in the same space
					but we have to prepare dummy b2Transform as argument for b2EdgeShape::RayCast
					*/
					b2Transform null_transform(b2Vec2(0.f, 0.f), b2Rot(0.f));

					return (marked_hole.RayCast(&output, input, null_transform, 0));
				}), discs_copy.end());
			}

			/* save all marked holes for pathfinding for example */
			response.marked_holes = marked_holes;
			/* save cleaned copy in actual discontinuities */
			response.discontinuities = discs_copy;
		}

		if (DEBUG_DRAWING.draw_discontinuities) {
			for (const auto& disc : response.discontinuities) {
				lines.emplace_back(disc.points.first, disc.points.second, rgba(0, 127, 255, 255));
			}
		}

		vis_responses.push_back(response);
	}

	ensure_eq(los_requests.size(), los_responses.size());
	ensure_eq(vis_requests.size(), vis_responses.size());
}