#pragma once

struct camera_cone;
struct b2AABB;
struct b2Filter;
class b2Shape;

enum class query_callback_result {
	CONTINUE,
	ABORT
};

template <class derived>
class physics_queries {
public:
	template<class F>
	void for_each_in_aabb_meters(
		const b2AABB aabb,
		const b2Filter filter,
		F callback
	) const {
		struct query_aabb_input : b2QueryCallback {
			b2Filter filter;
			F call;

			query_aabb_input(
				F&& f, 
				const b2Filter filter
			) : 
				call(f), 
				filter(filter) 
			{}

			bool ReportFixture(b2Fixture* fixture) override {
				if (b2ContactFilter::ShouldCollide(&filter, &fixture->GetFilterData())) {
					return call(fixture) == query_callback_result::CONTINUE;
				}

				return true;
			}
		};

		auto in = query_aabb_input(std::move(callback), filter);

		const auto& b2world = static_cast<const derived*>(this)->get_b2world();
		b2world.QueryAABB(&in, aabb);
	}

	template<class F>
	void for_each_intersection_with_shape_meters(
		const b2Shape* const shape,
		const b2Transform queried_shape_transform,
		const b2Filter filter,
		F callback
	) const {
		b2AABB shape_aabb;
		
		constexpr auto child_index = 0;

		shape->ComputeAABB(
			&shape_aabb, 
			queried_shape_transform,
			child_index
		);

		for_each_in_aabb_meters(
			shape_aabb,
			filter,
			[&](const b2Fixture* const fixture) -> query_callback_result {
				constexpr auto index_a = 0;
				constexpr auto index_b = 0;

				const auto result = b2TestOverlapInfo(
					shape,
					index_a,
					fixture->GetShape(),
					index_b,
					queried_shape_transform,
					fixture->GetBody()->GetTransform()
				);

				if (result.overlap) {
					return callback(
						fixture,
						METERS_TO_PIXELSf * result.pointA,
						METERS_TO_PIXELSf * result.pointB
					);
				}
				else {
					return query_callback_result::CONTINUE;
				}
			}
		);
	}

	template<class F>
	void for_each_in_aabb(
		const vec2 p1,
		const vec2 p2,
		const b2Filter filter,
		F callback
	) const {
		b2AABB aabb;
		aabb.lowerBound = p1 * PIXELS_TO_METERSf;
		aabb.upperBound = p2 * PIXELS_TO_METERSf;

		for_each_in_aabb_meters(aabb, filter, callback);
	}

	template<class F>
	void for_each_in_camera(
		const camera_cone camera,
		F callback
	) const {
		const auto visible_aabb = camera.get_transformed_visible_world_area_aabb().expand_from_center({ 100, 100 });

		for_each_in_aabb(
			visible_aabb.left_top(),
			visible_aabb.right_bottom(),
			filters::renderable_query(),
			callback
		);
	}

	template<class F>
	void for_each_intersection_with_body(
		const const_entity_handle subject,
		const b2Filter filter,
		F callback
	) const {
		const auto& self = *static_cast<const derived*>(this);
		const auto& cache = self.get_rigid_body_cache(subject);

		for (const b2Fixture* f = cache.body->GetFixtureList(); f != nullptr; f = f->GetNext()) {
			for_each_intersection_with_shape_meters(
				f->GetShape(),
				cache.body->GetTransform(),
				filter, 
				callback
			);
		}
	}

	template<class F>
	void for_each_intersection_with_triangle(
		const std::array<vec2, 3> vertices,
		const b2Filter filter,
		F callback
	) const {
		const b2Transform null_transform(b2Vec2(0.f, 0.f), b2Rot(0.f));
		b2PolygonShape poly_shape;

		std::array<b2Vec2, 3> verts;

		verts[0] = vertices[0] * PIXELS_TO_METERSf;
		verts[1] = vertices[1] * PIXELS_TO_METERSf;
		verts[2] = vertices[2] * PIXELS_TO_METERSf;

		poly_shape.Set(verts.data(), verts.size());

		for_each_intersection_with_shape_meters(
			&poly_shape,
			null_transform,
			filter, 
			callback
		);
	}

	template<class F>
	void for_each_intersection_with_polygon(
		const std::vector<vec2>& vertices,
		const b2Filter filter,
		F callback
	) const {
		const b2Transform null_transform(b2Vec2(0.f, 0.f), b2Rot(0.f));
		b2PolygonShape poly_shape;
		std::vector<b2Vec2> verts;

		for (const auto v : vertices) {
			verts.push_back(PIXELS_TO_METERSf * b2Vec2(v.x, v.y));
		}

		poly_shape.Set(verts.data(), verts.size());
		
		for_each_intersection_with_shape_meters(
			&poly_shape,
			null_transform,
			filter, 
			callback
		);
	}
};

inline auto get_id_of_entity_of_body(const b2Fixture* const f) {
	return f->GetBody()->GetUserData();
}

inline auto get_id_of_entity_of_fixture(const b2Fixture* const f) {
	return f->GetUserData();
}