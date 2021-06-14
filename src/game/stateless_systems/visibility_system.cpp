#include "augs/math/repro_math.h"

#include <limits>
#include <unordered_set>

#include "3rdparty/Box2D/Box2D.h"

#include "augs/math/math.h"
#include "augs/templates/container_templates.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/misc/simple_pair.h"
#include "game/detail/physics/physics_queries.h"
#include "game/debug_drawing_settings.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"

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
#include "application/performance_settings.h"

using namespace augs;
using namespace messages;

#define LOG_VISIBILITY 0

template <class... Args>
void VIS_LOG(Args&&... args) {
#if LOG_VISIBILITY
	LOG(std::forward<Args>(args)...);
#else
	((void)args, ...);
#endif
}

#if LOG_VISIBILITY
#define VIS_LOG_NVPS LOG_NVPS
#else
#define VIS_LOG_NVPS VIS_LOG
#endif

/*
	Thanks to:
	https://stackoverflow.com/questions/16542042/fastest-way-to-sort-vectors-by-angle-without-actually-computing-that-angle
*/

FORCE_INLINE auto comparable_angle(const vec2 diff) {
	return repro::copysignf(
		1 - diff.x / (repro::fabs(diff.x) + repro::fabs(diff.y)), diff.y 
	);
}

using edge = visibility_information_response::edge;
using discontinuity = visibility_information_response::discontinuity;
using triangle = visibility_information_response::triangle;

size_t visibility_information_response::get_num_triangles() const {
	return edges.size();
}

void visibility_information_response::clear() {
	source_queried_rect = {};
	edges.clear();
	vertex_hits.clear();
	discontinuities.clear();
	marked_holes.clear();
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
			repro::isnan(edges[i].first.x) ||
			repro::isnan(edges[i].first.y) ||
			repro::isnan(edges[i].second.x) ||
			repro::isnan(edges[i].second.y)
		) {
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

struct target_vertex {
	real32 angle;
	vec2 pos;
	bool is_on_a_bound;
	int vision_extends = 0;
	real32 dist_sq;

	bool operator<(const target_vertex& b) const {
		const auto diff = angle - b.angle;

		if (augs::is_epsilon(diff, 0.00001f)) {
			return dist_sq > b.dist_sq;
		}

		return diff < 0;
	}

	bool operator==(const target_vertex& b) const {
		return pos.compare(b.pos);
	}
};

bool visibility_information_request::valid() const {
	return queried_rect.x > 1.f && queried_rect.y > 1.f;
}

void visibility_system::calc_visibility(
	const cosmos& cosm,
	const visibility_request& request,
	visibility_response& response
) const {
	const auto si = cosm.get_si();

	const auto vtx_hit_col = yellow;
	const auto ray_obstructed_col = red;
	const auto extended_vision_hit_col = blue;
	const auto discontinuity_col = rgba(0, 127, 255, 255);
	const auto vis_rect_col = rgba(white).mult_alpha(0.7f);
	const auto free_area_col = pink;
	const auto triangle_edge_col = violet;
	const auto unreachable_area_col = white;

	const auto settings = [&cosm](){ 
		auto absolutize = [](float& f) {
			f = repro::fabs(f);
		};

		auto s = cosm.get_common_significant().visibility;

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
	const auto& physics = cosm.get_solvable_inferred().physics;

	struct ray_input {
		vec2 destination;
	};

	using ray_output = physics_raycast_output;

	const auto ignored_entity = request.subject;
	const auto transform = request.eye_transform;

	response.clear();
	response.source_queried_rect = request.queried_rect;

	if (!request.valid()) {
		return;
	}

	thread_local std::vector <target_vertex> all_vertices_transformed;
	all_vertices_transformed.clear();

	/* transform entity position to Box2D coordinates and take offset into account */
	const vec2 eye_meters = si.get_meters(transform.pos + request.offset);

	/* to Box2D coordinates */
	const auto vision_meters = si.get_meters(request.queried_rect);

	/* prepare maximum visibility square */
	b2AABB aabb;
	aabb.lowerBound = b2Vec2(eye_meters - vision_meters / 2);
	aabb.upperBound = b2Vec2(eye_meters + vision_meters / 2);

	auto push_vertex_if_within_range = [
		eye_meters, 
		boundary = ltrb(aabb.lowerBound.x, aabb.lowerBound.y, aabb.upperBound.x, aabb.upperBound.y)
	](const vec2 v) -> target_vertex* {
		if (!boundary.hover(vec2(v))) {
			return nullptr;
		}

		target_vertex new_vertex;
		new_vertex.pos = v;

		const auto diff = new_vertex.pos - eye_meters;

		new_vertex.angle = comparable_angle(diff);
		new_vertex.is_on_a_bound = false;
		new_vertex.dist_sq = diff.length_sq();

		all_vertices_transformed.push_back(new_vertex);
		return std::addressof(all_vertices_transformed.back());
	};

	auto push_vertex_on_boundary = [&](const vec2 v) {
		target_vertex new_vertex;
		new_vertex.pos = v;

		const auto diff = new_vertex.pos - eye_meters;

		new_vertex.angle = comparable_angle(diff);
		new_vertex.is_on_a_bound = true;
		new_vertex.dist_sq = diff.length_sq();

		all_vertices_transformed.push_back(new_vertex);
	};

	thread_local std::unordered_set<vec2i> surely_invisible_positions;
	surely_invisible_positions.clear();

	const auto invisible_eps = si.get_meters(settings.epsilon_distance_vertex_hit);

	auto add_surely_invisible = [invisible_eps](const vec2 v) {
		const auto x = static_cast<int>(repro::round(v.x / invisible_eps));
		const auto y = static_cast<int>(repro::round(v.y / invisible_eps));

		surely_invisible_positions.emplace(x, y);
	};

	auto is_invisible = [invisible_eps](const vec2 v) {
		const auto x = static_cast<int>(repro::round(v.x / invisible_eps));
		const auto y = static_cast<int>(repro::round(v.y / invisible_eps));

		return found_in(surely_invisible_positions, vec2i(x, y));
	};

	/* for every fixture that intersected with the visibility square */
	physics.for_each_in_aabb_meters(
		aabb, 
		request.filter,
		[&](const b2Fixture& f) {
			if (get_body_entity_that_owns(f) == Userdata(ignored_entity)) {
				return callback_result::CONTINUE;
			}

			const auto& shape = *f.m_shape;
			const auto xf = f.GetBody()->GetTransform();
			const auto eye_local = vec2(b2MulT(xf.q, eye_meters.operator b2Vec2() - xf.p));

			if (shape.GetType() == b2Shape::e_polygon) {
				const auto& poly = static_cast<const b2PolygonShape&>(shape);

				std::array<bool, b2_maxPolygonVertices> invisible_edges = {};

				const auto vn = poly.GetVertexCount();

				for (int vp = 0; vp < vn; ++vp) {
					const auto this_idx = vp;
					const auto next_idx = (vp + 1) % vn;

					const auto vert = poly.GetVertex(this_idx);
					const auto next_vert = poly.GetVertex(next_idx);

					const auto side = (eye_local - vert).cross(next_vert - vert);

					if (augs::is_zero(side)) {
						/* A collinear edge. The closer vertex is always visible, the further is not. */

						if (eye_local - vec2(vert) < eye_local - vec2(next_vert)) {
							VIS_LOG("Collinear and visible:");
							invisible_edges[next_idx] = true;
						}
						else {
							VIS_LOG("Collinear and invisible:");
							invisible_edges[this_idx] = true;
						}
					}
					else {
						if (side > 0.f) {
							VIS_LOG("Visible (%x):", side);
						}
						else {
							VIS_LOG("Invisible (%x):", side);
							invisible_edges[this_idx] = true;
						}
					}

					VIS_LOG_NVPS(vec2(vert), vp, invisible_edges[vp]);
				}

				auto idx = [vn](int i) {
					return i < 0 ? vn + i : i % vn;
				};

				VIS_LOG("Setting visions");

				for (int vp = 0; vp < vn; ++vp) {
					const auto vert = poly.GetVertex(vp);
					const auto vv = static_cast<vec2>(b2Mul(xf, vert));

					const bool this_vis = !invisible_edges[vp];
					const bool prev_vis = !invisible_edges[idx(vp - 1)];

					VIS_LOG_NVPS(prev_vis, this_vis);

					if (!this_vis && !prev_vis) {
						VIS_LOG("Surely invisible");
						add_surely_invisible(vv);
						continue;
					}

					if (const auto entry = push_vertex_if_within_range(vv)) {
						if (prev_vis && this_vis) {
							entry->vision_extends = 0;
						}
						else if (prev_vis && !this_vis) {
							entry->vision_extends = -1;
						}
						else if (!prev_vis && this_vis) {
							entry->vision_extends = 1;
						}

						VIS_LOG("Pushed %x", vp);
						VIS_LOG_NVPS(si.get_pixels(vv), entry->vision_extends);
					}
				}
			}

			return callback_result::CONTINUE;
		}
	);

	erase_if(
		all_vertices_transformed,
		[&](const auto& v) {
			return is_invisible(v.pos);
		}
	);

	VIS_LOG_NVPS(all_vertices_transformed.size());

	const auto visibility_bounds = [&](){
		/* extract the actual vertices from visibility AABB to cast rays to */
		const b2Vec2 whole_vision[] = {
			aabb.lowerBound,
			aabb.lowerBound + b2Vec2(vision_meters.x, 0),
			aabb.upperBound,
			aabb.upperBound - b2Vec2(vision_meters.x, 0)
		};

		/* prepare edge shapes given above vertices to cast rays against when no obstacle was hit
		note we lengthen them a bit and add/substract 1.f to avoid undeterministic vertex cases
		*/
		std::array<b2EdgeShape, 4> b;

		const auto moving_epsilon = si.get_meters(1.f);
		b[0].Set(b2Vec2(whole_vision[0]) + b2Vec2(-moving_epsilon, 0.f), b2Vec2(whole_vision[1]) + b2Vec2(moving_epsilon, 0.f));
		b[1].Set(b2Vec2(whole_vision[1]) + b2Vec2(0.f, -moving_epsilon), b2Vec2(whole_vision[2]) + b2Vec2(0.f, moving_epsilon));
		b[2].Set(b2Vec2(whole_vision[2]) + b2Vec2(moving_epsilon, 0.f), b2Vec2(whole_vision[3]) + b2Vec2(-moving_epsilon, 0.f));
		b[3].Set(b2Vec2(whole_vision[3]) + b2Vec2(0.f, moving_epsilon), b2Vec2(whole_vision[0]) + b2Vec2(0.f, -moving_epsilon));

		/* raycast through the bounds to add another vertices where the shapes go beyond visibility square */
		for (const auto& bound : b) {
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
				lines.emplace_back(si.get_pixels(bound.m_vertex1), si.get_pixels(bound.m_vertex2), vis_rect_col);
			}

			for (const auto& v : output) {
				push_vertex_on_boundary(v);
			}
		}

		/* add the visibility square to the vertices that we cast rays to, computing comparable angle in place */
		for (const auto& v : whole_vision) {
			push_vertex_on_boundary(v);
		}

		return b;
	}();

	sort_range(all_vertices_transformed);
	remove_duplicates_from_sorted(all_vertices_transformed);

#if LOG_VISIBILITY
	for (const auto& v : all_vertices_transformed) {
		VIS_LOG_NVPS(v.angle, si.get_pixels(v.pos), v.is_on_a_bound, v.vision_extends);
	}
#endif

	/* 
		all_vertices_transformed cannot be empty by now, since the boundary itself has four vertices.

		Debug line colors legend:

		red ray - ray that intersected with obstacle, these are ignored
		yellow ray - ray that hit the same vertex
		violet ray - ray that passed through vertex and hit another obstacle
		blue ray - ray that passed through vertex and hit boundary
	*/

	/* double_ray pair for holding both left-epsilon and right-epsilon rays */
	struct double_ray {
		vec2 first;
		vec2 second;

		double_ray() = default;

		double_ray(const vec2 first, const vec2 second) : 
			first(first),
			second(second)
		{
		}

		double_ray(const vec2 first) : 
			first(first),
			second(first)
		{
		}
	};

	/* container for these */
	thread_local std::vector<double_ray> double_rays;
	double_rays.clear();

	const auto push_double_ray = [&](auto... args) -> bool {
		const auto ray_b = double_ray(args...);
		const auto p2 = si.get_pixels(ray_b.first);

		VIS_LOG_NVPS(si.get_pixels(ray_b.first), si.get_pixels(ray_b.second));

		if (!double_rays.empty()) {
			const auto p1 = si.get_pixels(double_rays.back().first);

			if (p1.compare(p2)) {
				//return false;
			}
		}

		/* save new double_ray if it is not degenerate */
		if (repro::isfinite(p2.x) || repro::isfinite(p2.y)) {
			double_rays.push_back(ray_b);
			return true;
		}

		return false;
	};

	/* helper debugging lambda */
	const auto draw_line = [&](const vec2 point, const rgba col) {
		lines.emplace_back(si.get_pixels(eye_meters), si.get_pixels(point), col);
	};

	thread_local std::vector<ray_input> all_ray_inputs;

	all_ray_inputs.clear();
	all_ray_inputs.reserve(all_vertices_transformed.size());

	/* for every vertex to cast the ray to */
	for (const auto& vertex : all_vertices_transformed) {
		/* calculate the perpendicular direction to properly apply epsilon_ray_distance_variation */
		const auto perp = (vertex.pos - eye_meters).normalize().perpendicular_cw();

		const auto direction = [&]() {
			vec2 result;

			if (!vertex.vision_extends) {
				result = (vertex.pos - eye_meters).normalize();
			}
			else {
				const auto perp_offset = perp * vertex.vision_extends * si.get_meters(settings.epsilon_ray_distance_variation);
				const auto shifted_vert = vertex.pos + perp_offset;

				result = (shifted_vert - eye_meters).normalize();
			}

			return result;
		}();

		vec2 destination = eye_meters + direction * vision_meters.bigger_side() / 2 * 1.5f;

		for (const auto& bound : visibility_bounds) {
			const auto edge_ray_output = segment_segment_intersection(
				eye_meters, 
				destination, 
				bound.m_vertex1, 
				bound.m_vertex2
			);

			if (edge_ray_output.hit) {
				/* Clamp the ray against the visibility square */
				destination = edge_ray_output.intersection + direction * si.get_meters(3.f);
				break;
			}
		}

		ray_input new_ray_input;
		new_ray_input.destination = vertex.is_on_a_bound ? vertex.pos : destination;

#if LOG_VISIBILITY
		{
			const auto i = index_in(all_vertices_transformed, vertex);
			VIS_LOG_NVPS(i, si.get_pixels(new_ray_input.destination), vertex.is_on_a_bound);
		}
#endif

		all_ray_inputs.push_back(new_ray_input);
	}

	thread_local std::vector<ray_output> all_ray_outputs;

	all_ray_outputs.clear();
	all_ray_outputs.reserve(all_vertices_transformed.size());

	/* All raycast inputs are processed at once to improve cache coherency. */
	for (std::size_t j = 0; j < all_ray_inputs.size(); ++j) {
		auto result = physics.ray_cast(eye_meters, all_ray_inputs[j].destination, request.filter, ignored_entity);
		all_ray_outputs.emplace_back(std::move(result));

#if LOG_VISIBILITY
		if (DEBUG_DRAWING.draw_cast_rays) {
			draw_line(all_ray_inputs[j].destination, pink);
		}
#endif
	}

	for (std::size_t i = 0; i < all_ray_outputs.size(); ++i) {
		const auto& ray_callback = all_ray_outputs[i];
		auto& vertex = all_vertices_transformed[i];

		VIS_LOG_NVPS(i);
		/* const b2Vec2* from_aabb = nullptr; */

		/* if the vertex comes from bounding square, save it and remember about it */
		/* for (const auto& aabb_vert : whole_vision) { */
		/* 	if (vertex.pos == aabb_vert) { */
		/* 		from_aabb = &aabb_vert; */
		/* 	} */
		/* } */

		if (vertex.is_on_a_bound && (!ray_callback.hit || (ray_callback.intersection - vertex.pos).length_sq() < epsilon_distance_vertex_hit_sq)) {
			/* if it is a vertex on the boundary, handle it accordingly - interpret it as a new discontinuity (e.g. for pathfinding) */
			discontinuity new_discontinuity;
			new_discontinuity.edge_index = static_cast<int>(double_rays.size());
			new_discontinuity.points.first = vertex.pos;

			vec2 actual_normal;// = (ray_callbacks[0].normal / 2;
			vec2 actual_intersection;// = (+ray_callbacks[1].intersection) / 2;
			new_discontinuity.points.second = ray_callback.intersection;
			new_discontinuity.normal = actual_normal;

			new_discontinuity.winding = actual_normal.cross(eye_meters - actual_intersection) > 0 ?
				discontinuity::RIGHT : discontinuity::LEFT;

			/* if it is clockwise, we take previous edge as subject */
			if (new_discontinuity.winding == discontinuity::RIGHT) {
				--new_discontinuity.edge_index;
			}

			new_discontinuity.is_boundary = true;
			//request.discontinuities.push_back(new_discontinuity);
			VIS_LOG("on bound");
			if (push_double_ray(vertex.pos)) {
				if (DEBUG_DRAWING.draw_cast_rays) {
					draw_line(vertex.pos, vtx_hit_col);
				}
			}
		}
		else if (!vertex.is_on_a_bound) {
			if (ray_callback.hit) {
				/* 
					If distance between intersection and position is less than distance from target to position
					then ray must have intersected with an obstacle BEFORE reaching the vertex, ignoring intersection completely 
				*/
				const auto distance_from_origin = (vertex.pos - eye_meters).length();

				if ((ray_callback.intersection - eye_meters).length() + epsilon_threshold_obstacle_hit_meters < distance_from_origin) {
					VIS_LOG("obstructed");
					if (DEBUG_DRAWING.draw_cast_rays) {
						draw_line(vertex.pos, ray_obstructed_col);
					}
				}
				else if ((ray_callback.intersection - vertex.pos).length_sq() < epsilon_distance_vertex_hit_sq) {
					VIS_LOG("vtx hit");
					if (push_double_ray(vertex.pos)) {
						response.vertex_hits.emplace_back(
							static_cast<int>(double_rays.size()) - 1, 
							si.get_pixels(vertex.pos)
						);

						if (DEBUG_DRAWING.draw_cast_rays) {
							draw_line(vertex.pos, vtx_hit_col);
						}
					}
				}
				else if (vertex.vision_extends) {
					/* 
						Here: ray reached the target or even further (guaranteed by first condition),
						and they are not close to each other (guaranteed by condition 2),
						so either one of them hit the vertex and the second one went further or we have a pathological case
						that both went further and still intersected somewhere close - this is something we either way cannot handle and occurs
						when a body is pathologically thin

						so this is the case where the ray is cast at the lateral vertex,
						here we also detect the discontinuity 
					*/

					double_ray new_double_ray; 

					discontinuity new_discontinuity;
					new_discontinuity.points.first = vertex.pos;
					new_discontinuity.points.second = ray_callback.intersection;

					if (vertex.vision_extends == -1) {
						new_double_ray.first = ray_callback.intersection;
						new_double_ray.second = vertex.pos;
						new_discontinuity.winding = discontinuity::LEFT;
						new_discontinuity.edge_index = static_cast<int>(double_rays.size());
					}
					else if (vertex.vision_extends == 1) {
						new_double_ray.first = vertex.pos;
						new_double_ray.second = ray_callback.intersection;
						new_discontinuity.winding = discontinuity::RIGHT;
						new_discontinuity.edge_index = static_cast<int>(double_rays.size() - 1);
					}

					if (DEBUG_DRAWING.draw_cast_rays) {
						draw_line(ray_callback.intersection, free_area_col);
					}

					/* save new double ray */
					VIS_LOG("disc");
					if (push_double_ray(new_double_ray)) {
						/* save new discontinuity */
						response.discontinuities.push_back(new_discontinuity);

						if (DEBUG_DRAWING.draw_cast_rays && !DEBUG_DRAWING.draw_discontinuities) {
							draw_line(ray_callback.intersection, discontinuity_col);
							draw_line(vertex.pos, discontinuity_col);
						}
					}
				}
				else {
					/*
						This would happen if the vertex that has extend vision = 0
						is almost collinear with the eye and the vertex that begins to have a non-zero extended vision

						We choose to register it as a vertex hit
					*/

					VIS_LOG("collinear vtx hit");
					if (push_double_ray(vertex.pos)) {
						response.vertex_hits.emplace_back(
							static_cast<int>(double_rays.size()) - 1, 
							si.get_pixels(vertex.pos)
						);

						if (DEBUG_DRAWING.draw_cast_rays) {
							draw_line(vertex.pos, vtx_hit_col);
						}
					}
				}
			}
			else {
				/* 
					If the ray didn't hit anything, it must have reached the boundary.
					Of course, we also consider this a discontinuity.
				*/

				/* For every edge from 4 edges forming visibility square */
				for (const auto& bound : visibility_bounds) {
					const auto ray_edge_output = segment_segment_intersection(
						eye_meters, 
						all_ray_inputs[i].destination,
						bound.m_vertex1, 
						bound.m_vertex2
					);

					/* if we hit against boundaries (must happen for at least 1 of them) */
					if (ray_edge_output.hit) {
						const auto boundary_intersection = ray_edge_output.intersection;

						if (vertex.vision_extends) {
							/* prepare new discontinuity data */
							discontinuity new_discontinuity;

							/* compute the actual intersection point from b2RayCastOutput data */

							new_discontinuity.points.first = vertex.pos;
							new_discontinuity.points.second = boundary_intersection;

							double_ray new_double_ray;

							if (vertex.vision_extends == -1) {
								new_discontinuity.winding = discontinuity::LEFT;
								new_discontinuity.edge_index = static_cast<int>(double_rays.size());
								new_double_ray = double_ray(boundary_intersection, vertex.pos);
							}
							else if (vertex.vision_extends == 1) {
								new_discontinuity.winding = discontinuity::RIGHT;
								new_discontinuity.edge_index = static_cast<int>(double_rays.size()) - 1;
								new_double_ray = double_ray(vertex.pos, boundary_intersection);
							}

							/* save new double ray */
							VIS_LOG("boundary hit: discontinuity");
							if (push_double_ray(new_double_ray)) {
								response.discontinuities.push_back(new_discontinuity);

								if (DEBUG_DRAWING.draw_cast_rays) {
									draw_line(boundary_intersection, extended_vision_hit_col);
								}
							}
						}
						else {
							VIS_LOG("boundary hit: vtx hit");
							if (push_double_ray(vertex.pos)) {
								response.vertex_hits.emplace_back(
									static_cast<int>(double_rays.size()) - 1, 
									si.get_pixels(vertex.pos)
								);

								if (DEBUG_DRAWING.draw_cast_rays) {
									draw_line(vertex.pos, vtx_hit_col);
								}
							}
						}
					}
				}
			}
		}
	}

	/* now propagate the output */
	for (std::size_t i = 0; i < double_rays.size(); ++i) {
		/* (i + 1)%double_rays.size() ensures the cycle */
		const auto& ray_a = double_rays[i];
		const auto& ray_b = double_rays[(i + 1) % double_rays.size()];

		/* transform intersection locations to pixels */
		vec2 p1 = si.get_pixels(ray_a.second);
		vec2 p2 = si.get_pixels(ray_b.first);

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
		auto& marked_holes = response.marked_holes;

		/* for every discontinuity, remove if there exists any edge that is too close to the discontinuity's vertex */
		discs_copy.erase(std::remove_if(discs_copy.begin(), discs_copy.end(),
			[unreachable_area_col, &request, edges_num, &transform, &wrap, &lines, &marked_holes, &response]
		(const discontinuity& d) {
			thread_local std::vector<vec2> points_too_close;
			points_too_close.clear();

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
				marked_holes.emplace_back(edge(closest_point, d.points.first));

				if (DEBUG_DRAWING.draw_discontinuities) {
					lines.emplace_back(unreachable_area_col, closest_point, d.points.first);
				}

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
				b2Transform null_transform;
				null_transform.SetIdentity();

				return (marked_hole.RayCast(&output, input, null_transform, 0));
			}), discs_copy.end());
		}

		/* save cleaned copy in actual discontinuities */
		response.discontinuities = std::move(discs_copy);
	}

	if (DEBUG_DRAWING.draw_discontinuities) {
		for (const auto& disc : response.discontinuities) {
			lines.emplace_back(disc.points.first, disc.points.second, discontinuity_col);
		}
	}

	if (DEBUG_DRAWING.draw_triangle_edges) {
		const auto& r = response;

		for (std::size_t t = 0; t < r.get_num_triangles(); ++t) {
			if (time(NULL) % r.get_num_triangles() != t) {
				continue;
			}

			const auto world_light_tri = r.get_world_triangle(t, si.get_pixels(eye_meters));
			auto col = triangle_edge_col;
			col.set_hsv({ t * 0.05f, 1.f, 1.f });

			lines.emplace_back(world_light_tri[0], world_light_tri[1], col);
			lines.emplace_back(world_light_tri[1], world_light_tri[2], col);
			lines.emplace_back(world_light_tri[2], world_light_tri[0], col);
		}
	}
}
