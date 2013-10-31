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
	entity* subject;
	vec2<> intersection, target;
	bool hit;

	my_ray_callback() : hit(false) {}

	float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point,
		const b2Vec2& normal, float32 fraction) override {
			if ((entity*) fixture->GetBody()->GetUserData() == subject)
			{
				return -1;
			}

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

ai_system::ai_system() : draw_cast_rays(false), draw_triangle_edges(true), draw_discontinuities(false) {}

int components::ai::get_num_triangles() {
	return vision_points.size() - 1;
}

components::ai::triangle components::ai::get_triangle(int i, augmentations::vec2<> origin) {
	components::ai::triangle tri = { origin, vision_points[i], vision_points[i+1] };
	return tri;
}

void ai_system::process_entities(world& owner) {
	physics_system& physics = owner.get_system<physics_system>();

	for (auto it : targets) {
		/* prepare container for all the vertices that we will cast the ray to */
		static std::vector < std::pair < float, vec2< >> > all_vertices_transformed;
		all_vertices_transformed.clear();

		/* get AI data and position of the entity */
		auto& ai = it->get<components::ai>();
		auto& transform = it->get<components::transform>().current;

		/* transform entity position and its visibility radius to Box2D coordinates */
		vec2<> position_meters = transform.pos * PIXELS_TO_METERSf;
		float vision_side_meters = ai.visibility_square_side * PIXELS_TO_METERSf;

		/* prepare maximum visibility square */
		b2AABB aabb;
		aabb.lowerBound = position_meters - vision_side_meters/2;
		aabb.upperBound = position_meters + vision_side_meters/2;
		
		/* get list of all fixtures that intersect with the visibility square */
		my_callback callback;
		callback.subject = it;
		physics.b2world.QueryAABB(&callback, aabb);

		/* for every fixture that intersected with the visibility square */
		for (auto fixture : callback.fixtures) {
			auto shape = fixture->GetShape();
			
			/* if it is polygonal shape (we don't support other shapes for the moment) */
			if (shape->GetType() == b2Shape::e_polygon) {
				auto polygon_shape = static_cast<b2PolygonShape*>(shape);

				int verts = polygon_shape->GetVertexCount();
				
				/* for every vertex in given fixture's shape */
				for (int i = 0; i < verts; ++i) {
					auto position = fixture->GetBody()->GetPosition();
					/* transform angle to degrees */
					auto rotation = fixture->GetBody()->GetAngle() / 0.01745329251994329576923690768489;
					
					std::pair<float, vec2<>> new_vertex;
					/* transform vertex to current entity's position and rotation */
					new_vertex.second = vec2<>(polygon_shape->GetVertex(i)).rotate(rotation, b2Vec2(0, 0)) + position;

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
			ai.lines.push_back(components::ai::debug_line(vec2<>(whole_vision[0]) + vec2<>(-1.f, 0.f), vec2<>(whole_vision[1]) + vec2<>(1.f, 0.f)));
			ai.lines.push_back(components::ai::debug_line(vec2<>(whole_vision[1]) + vec2<>(0.f, -1.f), vec2<>(whole_vision[2]) + vec2<>(0.f, 1.f)));
			ai.lines.push_back(components::ai::debug_line(vec2<>(whole_vision[2]) + vec2<>(1.f, 0.f), vec2<>(whole_vision[3]) + vec2<>(-1.f, 0.f)));
			ai.lines.push_back(components::ai::debug_line(vec2<>(whole_vision[3]) + vec2<>(0.f, 1.f), vec2<>(whole_vision[0]) + vec2<>(0.f, -1.f)));
		}
	
		/* add the visibility square to the vertices that we cast rays to, computing comparable angle in place */
		for (auto& v : whole_vision)
			all_vertices_transformed.push_back(std::make_pair(comparable_angle(v - position_meters), v));

		/* SORT ALL VERTICES BY ANGLE */
		std::sort(all_vertices_transformed.begin(), all_vertices_transformed.end());

		/* by now we have ensured that all_vertices_transformed is non-empty 
			this value is to be tuned
		*/
		const auto epsilon = 0.0001f;
			//std::numeric_limits<float>().epsilon();
		
		/*
		debugging:
			red ray - ray that intersected with obstacle, these are ignored
			yellow ray - ray that hit the same vertex
			violet ray - ray that passed through vertex and hit another obstacle
			blue ray - ray that passed through vertex and hit boundary
		*/

		/* double_ray pair for holding both left-epsilon and right-epsilon rays */
		typedef std::pair<vec2<>, vec2<>> double_ray;
		std::vector<double_ray> double_rays;
		
		/* debugging lambda */
		auto draw_line = [&position_meters, &ai](vec2<> point, graphics::pixel_32 col){
			ai.lines.push_back(components::ai::debug_line(position_meters, point, col));
		};

		/* container for holding info about local discontinuities, will be used for dynamic AI navigation */
		std::vector<components::ai::discontinuity> local_discontinuities;

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
			ray_callbacks[1].subject = it;
			/* create a vector in direction of vertex with length equal to the half of diagonal of the visibility square
			(majority of rays will slightly SLIGHTLY go beyond visibility square, but that's not important for now)
			
			ray_callbacks[0] and ray_callbacks[1] differ ONLY by an epsilon added/substracted to the angle
			*/
			ray_callbacks[0].target = position_meters + vec2<>::from_degrees((vertex.second - position_meters).get_degrees() - epsilon) * vision_side_meters / 2 * 1.414213562373095;
			ray_callbacks[1].target = position_meters + vec2<>::from_degrees((vertex.second - position_meters).get_degrees() + epsilon) * vision_side_meters / 2 * 1.414213562373095;

			/* cast both rays starting from the player position and ending in ray_callbacks[x].target */
			physics.b2world.RayCast(&ray_callbacks[0], position_meters, ray_callbacks[0].target);
			physics.b2world.RayCast(&ray_callbacks[1], position_meters, ray_callbacks[1].target);
			
			/* if we did not intersect with anything */
			if (!(ray_callbacks[0].hit || ray_callbacks[1].hit)) {
				/* we must have cast the ray against AABB */
				if (from_aabb) {
					double_rays.push_back(double_ray(*from_aabb, *from_aabb));
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
				/* if distance of intersection is bigger than epsilon
				then ray must have intersected with an obstacle, ignoring intersection completely */
				if ((ray_callbacks[0].intersection - vertex.second).length_sq() > 0.001f &&
					(ray_callbacks[1].intersection - vertex.second).length_sq() > 0.001f) {
						if(draw_cast_rays) draw_line(ray_callbacks[0].intersection, graphics::pixel_32(255, 0, 0, 255));
				}
				/* distance between both intersections fit in epsilon which means ray intersected with the same vertex */
				else if ((ray_callbacks[0].intersection - ray_callbacks[1].intersection).length_sq() < 0.0001f) {
					/* interpret it as both rays hit the same vertex 
					for accuracy reasons, push the vertex coordinates instead of the actual intersections */
					double_rays.push_back(double_ray(vertex.second, vertex.second));
					if (draw_cast_rays) draw_line(vertex.second, graphics::pixel_32(255, 255, 0, 255));
				}
				/* this is the case where the ray is cast at the lateral vertex, 
					here we also detect the discontinuity */
				else {
					/* save both intersection points, this is what we introduced "double_ray" pair for */
					double_rays.push_back(double_ray(ray_callbacks[0].intersection, ray_callbacks[1].intersection));

					/* handle new discontinuity */
					components::ai::discontinuity new_discontinuity;

					/* if the ray that we substracted the epsilon from intersected closer (and thus with the vertex), then the free space is to the right */
					if ((ray_callbacks[0].intersection - position_meters).length_sq() < (ray_callbacks[1].intersection - position_meters).length_sq()) {
						new_discontinuity.p1 = ray_callbacks[0].intersection;
						new_discontinuity.p2 = ray_callbacks[1].intersection;
						new_discontinuity.winding = components::ai::discontinuity::RIGHT;
						if (draw_cast_rays) draw_line(ray_callbacks[1].intersection, graphics::pixel_32(255, 0, 255, 255));
					}
					/* otherwise it is to the left */
					else {
						new_discontinuity.p1 = ray_callbacks[1].intersection;
						new_discontinuity.p2 = ray_callbacks[0].intersection;
						new_discontinuity.winding = components::ai::discontinuity::LEFT;
						if (draw_cast_rays) draw_line(ray_callbacks[0].intersection, graphics::pixel_32(255, 0, 255, 255));
					}
					/* save new discontinuity */
					local_discontinuities.push_back(new_discontinuity);
				}
			}
			/* the case where exactly one of the rays did not hit anything so we cast it against boundaries, 
				we also detect discontinuity here */
			else {
				/* for every callback that didn't detect hit (it will be only one) */
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
							b2Transform edge_transform(b2Vec2(0, 0), b2Rot(0));

							/* if we hit against boundaries (must happen for at least 1 of them) */
							if (bound.RayCast(&output, input, edge_transform, 0)) {
								/* prepare new discontinuity data */
								components::ai::discontinuity new_discontinuity;

								/* compute the actual intersection point from b2RayCastOutput data */
								auto actual_intersection = input.p1 + output.fraction * (input.p2 - input.p1);
								/* if the left-handed ray intersected with boundary */
								if (i == 0) {
									new_discontinuity.p1 = ray_callbacks[1].intersection;
									new_discontinuity.p2 = ray_callbacks[0].intersection;
									new_discontinuity.winding = components::ai::discontinuity::LEFT;
									double_rays.push_back(double_ray(actual_intersection, ray_callbacks[1].intersection));
								}
								/* if the right-handed ray intersected with boundary */
								else if (i == 1) {
									new_discontinuity.p1 = ray_callbacks[0].intersection;
									new_discontinuity.p2 = ray_callbacks[1].intersection;
									new_discontinuity.winding = components::ai::discontinuity::RIGHT;
									double_rays.push_back(double_ray(ray_callbacks[0].intersection, actual_intersection));
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
		ai.vision_points.clear();
		
		if (draw_discontinuities)
			ai.memorised_discontinuities = local_discontinuities;

		for (int i = 0; i < double_rays.size(); ++i) {
			ai.vision_points.push_back(double_rays[i].second * METERS_TO_PIXELSf);
			if (draw_triangle_edges) draw_line(PIXELS_TO_METERSf*(*ai.vision_points.rbegin()), ai.visibility_color);
			/* (i + 1)%double_rays.size() ensures the cycle */
			ai.vision_points.push_back(double_rays[(i + 1)%double_rays.size()].first * METERS_TO_PIXELSf);
			if (draw_triangle_edges) draw_line(PIXELS_TO_METERSf*(*ai.vision_points.rbegin()), ai.visibility_color);

			if (draw_triangle_edges) ai.lines.push_back(components::ai::debug_line(PIXELS_TO_METERSf**ai.vision_points.rbegin(), PIXELS_TO_METERSf**(ai.vision_points.rbegin() + 1), ai.visibility_color));
		}
	}
}