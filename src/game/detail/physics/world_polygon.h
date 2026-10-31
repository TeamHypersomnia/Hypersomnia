#pragma once
#include <Box2D/Collision/Shapes/b2PolygonShape.h>
#include <Box2D/Dynamics/b2Fixture.h>
#include <Box2D/Dynamics/b2Body.h>

#include "augs/math/vec2.h"
#include "augs/math/si_scaling.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/math/polygon_queries.h"
#include "game/inferred_caches/find_physics_cache.h"

/*
	A polygon fixture's vertices in the world, in pixels - empty for any other shape.
*/

inline auto get_world_polygon_px(const b2Fixture& fixture, const si_scaling si) {
	augs::constant_size_vector<vec2, b2_maxPolygonVertices> result;

	const auto* const shape = fixture.GetShape();

	if (shape->GetType() != b2Shape::e_polygon) {
		return result;
	}

	const auto& poly = static_cast<const b2PolygonShape&>(*shape);
	const auto& xf = fixture.GetBody()->GetTransform();

	for (int v = 0; v < poly.GetVertexCount(); ++v) {
		result.push_back(si.get_pixels(static_cast<vec2>(b2Mul(xf, poly.GetVertex(v)))));
	}

	return result;
}

/*
	The polygons of all of a body's fixtures, in the world, in pixels - as the physics has them now.
*/

using body_polygons_px = augs::constant_size_vector<augs::constant_size_vector<vec2, b2_maxPolygonVertices>, 32>;

template <class E>
body_polygons_px get_body_polygons_px(const E& handle, const si_scaling si) {
	body_polygons_px result;

	if (const auto* const cache = ::find_colliders_cache(handle)) {
		for (const auto& fp : cache->constructed_fixtures) {
			if (result.size() == result.max_size()) {
				break;
			}

			const auto polygon = ::get_world_polygon_px(*fp.get(), si);

			if (polygon.size() >= 3) {
				result.push_back(polygon);
			}
		}
	}

	return result;
}
