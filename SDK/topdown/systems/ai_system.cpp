#include "stdafx.h"
#include "ai_system.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "physics_system.h"

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
	vec2<> intersection;
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

		if (callback.fixtures.empty()) {

		}

		for (auto fixture : callback.fixtures) {
			auto shape = fixture->GetShape();
			
			if (shape->GetType() == b2Shape::e_polygon) {
				auto polygon_shape = static_cast<b2PolygonShape*>(shape);

				int verts = polygon_shape->GetVertexCount();
				
				for (int i = 0; i < verts; ++i) {
					auto position = fixture->GetBody()->GetPosition();
					auto rotation = fixture->GetBody()->GetAngle();
					
					std::pair<float, vec2<>> new_vertex;
					new_vertex.second = vec2<>(polygon_shape->GetVertex(i)).rotate(rotation, fixture->GetBody()->GetWorldCenter()) + position;

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

		//for (auto& v : whole_vision)
		//	all_vertices_transformed.push_back(std::make_pair(comparable_angle(v), v));

		std::sort(all_vertices_transformed.begin(), all_vertices_transformed.end());

		/* now to propagate the visible area */
		ai.vision_points.clear();

		vec2<> last_vertex_that_ray_reached;
		/* by now we have ensured that all_vertices_transformed is non-empty */
		for (auto& vertex : all_vertices_transformed) {
			my_ray_callback ray_callback;

			auto normalized_direction = (vertex.second - position_meters).normalize();

			physics.b2world.RayCast(&ray_callback, position_meters, position_meters + (normalized_direction*vision_side_meters / 2 * 1.414213562373095));
			ai.lines.push_back(components::ai::debug_line(position_meters, position_meters + (normalized_direction*vision_side_meters / 2 * 1.414213562373095)));

			if (!ray_callback.hit) {
				for (auto& bound : bounds) {
					b2RayCastOutput output;
					b2RayCastInput input;
					input.maxFraction = 1.0;
					input.p1 = position_meters;
					input.p2 = position_meters + (normalized_direction * (vision_side_meters + 0.1) / 2 * 1.414213562373095);

					b2Transform edge_transform(b2Vec2(0, 0), b2Rot(0));
					ai.lines.push_back(components::ai::debug_line(input.p1, input.p2));

					if (bound.RayCast(&output, input, edge_transform, 0)) {
						//ray_callback.intersection = input.p2 + output.fraction * (input.p2 - input.p1);
						break;
					}
				}
			}

			//if ((ray_callback.intersection - position_meters).length_sq() >= (vertex.second - position_meters).length_sq()) {
				//ai.vision_points.push_back(last_vertex_that_ray_reached);
				ai.vision_points.push_back(ray_callback.intersection);

				//last_vertex_that_ray_reached = vertex.second;
			//}

			/* otherwise ray has not reached its destination, discard result to not create redundant triangles */
		}

		for (auto& vision_point : ai.vision_points) {
			vision_point *= METERS_TO_PIXELSf;
		}
	}
}