#include "stdafx.h"
#include "ai_system.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "physics_system.h"
#include <limits>

struct my_callback : public b2QueryCallback {
	std::vector<b2Fixture*> fixtures;
	entity* subject;

	bool ReportFixture(b2Fixture* fixture) override {
		if ((entity*)fixture->GetBody()->GetUserData() != subject)
			fixtures.push_back(fixture);
		return true;
	}
};

struct my_ray_callback : public b2RayCastCallback {
	vec2<> intersection, target;
	bool hit;

	my_ray_callback() : hit(false) {}

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

float comparable_angle(vec2<> diff) {
	return sgn(diff.y) * (
		1 - (diff.x / (std::abs(diff.x) + std::abs(diff.y)))
		);
}

int components::ai::get_num_triangles() {
	return vision_points.size() - 1;
}

components::ai::triangle components::ai::get_triangle(int i, augmentations::vec2<> origin) {
	components::ai::triangle tri = { origin, vision_points[i], vision_points[i+1] };
	return tri;
}

void ai_system::process_entities(world& owner) {
	physics_system& physics = owner.get_system<physics_system>();

	static std::vector<std::pair<float, vec2<>>> all_vertices_transformed;
	all_vertices_transformed.clear();

	for (auto it : targets) {
		auto& ai = it->get<components::ai>();
		auto& transform = it->get<components::transform>();

		vec2<> position_meters = transform.current.pos * PIXELS_TO_METERSf;
		float vision_side_meters = ai.visibility_square_side * PIXELS_TO_METERSf;

		b2AABB aabb;
		aabb.lowerBound = position_meters - vision_side_meters/2;
		aabb.upperBound = position_meters + vision_side_meters/2;
		
		my_callback callback;
		callback.subject = it;
		physics.b2world.QueryAABB(&callback, aabb);

		for (auto fixture : callback.fixtures) {
			auto shape = fixture->GetShape();
			
			if (shape->GetType() == b2Shape::e_polygon) {
				auto polygon_shape = static_cast<b2PolygonShape*>(shape);

				int verts = polygon_shape->GetVertexCount();
				
				for (int i = 0; i < verts; ++i) {
					auto position = fixture->GetBody()->GetPosition();
					auto rotation = fixture->GetBody()->GetAngle() / 0.01745329251994329576923690768489;
					
					std::pair<float, vec2<>> new_vertex;
					new_vertex.second = vec2<>(polygon_shape->GetVertex(i)).rotate(rotation, fixture->GetBody()->GetLocalCenter()) + position;

					vec2<> diff = new_vertex.second - position_meters;

					new_vertex.first = comparable_angle(diff);
					all_vertices_transformed.push_back(new_vertex);
				}
			}
		}

		/* vision AABB */
		b2Vec2 whole_vision [] = {
			aabb.lowerBound,
			aabb.lowerBound + vec2<>(vision_side_meters, 0),
			aabb.upperBound,
			aabb.upperBound - vec2<>(vision_side_meters, 0)
		};

		b2EdgeShape bounds[4];
		bounds[0].Set(vec2<>(whole_vision[0]) + vec2<>(-1.f, 0.f), vec2<>(whole_vision[1]) + vec2<>(1.f, 0.f));
		bounds[1].Set(vec2<>(whole_vision[1]) + vec2<>(0.f, -1.f), vec2<>(whole_vision[2]) + vec2<>(0.f, 1.f));
		bounds[2].Set(vec2<>(whole_vision[2]) + vec2<>(1.f, 0.f), vec2<>(whole_vision[3]) + vec2<>(-1.f, 0.f));
		bounds[3].Set(vec2<>(whole_vision[3]) + vec2<>(0.f, 1.f), vec2<>(whole_vision[0]) + vec2<>(0.f, -1.f));

		ai.lines.push_back(components::ai::debug_line(vec2<>(whole_vision[0]) + vec2<>(-1.f, 0.f), vec2<>(whole_vision[1]) + vec2<>(1.f, 0.f)));
		ai.lines.push_back(components::ai::debug_line(vec2<>(whole_vision[1]) + vec2<>(0.f, -1.f), vec2<>(whole_vision[2]) + vec2<>(0.f, 1.f)));
		ai.lines.push_back(components::ai::debug_line(vec2<>(whole_vision[2]) + vec2<>(1.f, 0.f), vec2<>(whole_vision[3]) + vec2<>(-1.f, 0.f)));
		ai.lines.push_back(components::ai::debug_line(vec2<>(whole_vision[3]) + vec2<>(0.f, 1.f), vec2<>(whole_vision[0]) + vec2<>(0.f, -1.f)));

		for (auto& v : whole_vision)
			all_vertices_transformed.push_back(std::make_pair(comparable_angle(v - position_meters), v));

		std::sort(all_vertices_transformed.begin(), all_vertices_transformed.end());

		/* by now we have ensured that all_vertices_transformed is non-empty */
		const auto epsilon = 0.0001f;
			//std::numeric_limits<float>().epsilon();
		/*
		debugging:
			red ray - ignored rays that intersected with obstacle
			yellow ray - hit the same vertex
			violet ray - ray that passed through vertex and hit another obstacle
			blue ray - ray that hit boundary and/or passed through vertex
		*/

		typedef std::pair<vec2<>, vec2<>> double_ray;
		std::vector<double_ray> double_rays;
		
		auto draw_line = [&position_meters, &ai](vec2<> point, graphics::pixel_32 col){
			ai.lines.push_back(components::ai::debug_line(position_meters, point, col));
		};

		for (auto& vertex : all_vertices_transformed) {
			b2Vec2* from_aabb = nullptr;

			for (auto& aabb_vert : whole_vision)
				if (vertex.second == aabb_vert) 
					from_aabb = &aabb_vert;
			
			my_ray_callback ray_callbacks[2];

			ray_callbacks[0].target = position_meters + vec2<>::from_angle((vertex.second - position_meters).get_degrees() - epsilon) * vision_side_meters / 2 * 1.414213562373095;
			ray_callbacks[1].target = position_meters + vec2<>::from_angle((vertex.second - position_meters).get_degrees() + epsilon) * vision_side_meters / 2 * 1.414213562373095;

			physics.b2world.RayCast(&ray_callbacks[0], position_meters, ray_callbacks[0].target);
			physics.b2world.RayCast(&ray_callbacks[1], position_meters, ray_callbacks[1].target);
			
			/*if it was cast against the aabb, there's a high probability we did not intersect with anything */
			if (!(ray_callbacks[0].hit || ray_callbacks[1].hit)) {
				if (from_aabb) 
					double_rays.push_back(double_ray(*from_aabb, *from_aabb));
				else {
					/* only ever happens if an object is partially inside visibility rectangle
					ignore, handling not implemented
					*/
					bool breakpoint = true;
					//draw_line(ray_callbacks[0].target, graphics::pixel_32(0, 255, 255, 255));
					//draw_line(ray_callbacks[1].target, graphics::pixel_32(0, 255, 0, 255));
				}
			}
			else if (ray_callbacks[0].hit && ray_callbacks[1].hit) {
				/* ray intersected with an obstacle, ignoring intersection */
				if ((ray_callbacks[0].intersection - vertex.second).length_sq() > 0.001f &&
					(ray_callbacks[1].intersection - vertex.second).length_sq() > 0.001f) {
						//draw_line(ray_callbacks[0].intersection, graphics::pixel_32(255, 0, 0, 255));
				}
				/* intersected with the same vertex */
				else if ((ray_callbacks[0].intersection - ray_callbacks[1].intersection).length_sq() < 0.0001f) {
					double_rays.push_back(double_ray(vertex.second, vertex.second));
					//draw_line(vertex.second, graphics::pixel_32(255, 255, 0, 255));
				}
				/* this is the case where the ray is cast at the peripheral vertex, here we also detect the discontinuity */
				else {
					double_rays.push_back(double_ray(ray_callbacks[0].intersection, ray_callbacks[1].intersection));

					if ((ray_callbacks[0].intersection - position_meters).length_sq() > (ray_callbacks[1].intersection - position_meters).length_sq()) {
					//	draw_line(ray_callbacks[0].intersection, graphics::pixel_32(255, 0, 255, 255));
					}
					else {
					//	draw_line(ray_callbacks[1].intersection, graphics::pixel_32(255, 0, 255, 255));
					}
				}
			}
			/* one of the rays did not hit anything so we cast it against boundaries, we also detect discontinuity here
			*/
			else {
				for (int i = 0; i < 2; ++i) {
					if (!ray_callbacks[i].hit) {
						for (auto& bound : bounds) {
							b2RayCastOutput output;
							b2RayCastInput input;
							input.maxFraction = 1.0;
							input.p1 = position_meters;
							input.p2 = ray_callbacks[i].target;

							b2Transform edge_transform(b2Vec2(0, 0), b2Rot(0));

							if (bound.RayCast(&output, input, edge_transform, 0)) {
								auto actual_intersection = input.p1 + output.fraction * (input.p2 - input.p1);
								if (i == 0) double_rays.push_back(double_ray(actual_intersection, ray_callbacks[1].intersection));
								else if (i == 1) double_rays.push_back(double_ray(ray_callbacks[0].intersection, actual_intersection));
								//draw_line(actual_intersection, graphics::pixel_32(0, 0, 255, 255));
							}
						}
						break;
					}
				}
			}
		}

		/* now to propagate the visible area */
		ai.vision_points.clear();

		for (int i = 0; i < double_rays.size(); ++i) {
			ai.vision_points.push_back(double_rays[i].second * METERS_TO_PIXELSf);
			//draw_line(PIXELS_TO_METERSf**ai.vision_points.rbegin(), graphics::pixel_32(255, 255, 255, 255));
			ai.vision_points.push_back(double_rays[(i + 1)%double_rays.size()].first * METERS_TO_PIXELSf);
			//draw_line(PIXELS_TO_METERSf**ai.vision_points.rbegin(), graphics::pixel_32(255, 255, 255, 255));

			//ai.lines.push_back(components::ai::debug_line(PIXELS_TO_METERSf**ai.vision_points.rbegin(), PIXELS_TO_METERSf**(ai.vision_points.rbegin() + 1), graphics::pixel_32(255, 255, 255, 255)));
		}
	}
}