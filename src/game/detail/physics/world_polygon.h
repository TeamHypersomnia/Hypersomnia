#pragma once
#include <Box2D/Collision/Shapes/b2PolygonShape.h>
#include <Box2D/Dynamics/b2Fixture.h>
#include <Box2D/Dynamics/b2Body.h>

#include "augs/math/vec2.h"
#include "augs/math/si_scaling.h"
#include "augs/misc/constant_size_vector.h"

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
