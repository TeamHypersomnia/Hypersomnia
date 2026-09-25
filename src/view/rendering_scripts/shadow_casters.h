#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#include "augs/math/vec2.h"
#include "augs/math/transform.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/components/render_component.h"
#include "game/components/rigid_body_component.h"

/*
	Helpers shared by the environment shadows and the shadows of lights with a height.

	Shadow heights are in pixels of height.
	cosmos_light_settings::shadow_step is how far the sun moves a shadow per one pixel of height.
*/

inline uint8_t calc_fixture_shadow_height(const cosmos& cosm, const b2Fixture& fix) {
	const auto owner = cosm[fix.GetUserData()];

	const auto base_height = [&]() {
		if (owner) {
			if (const auto render = owner.template find<invariants::render>()) {
				return render->shadow_height;
			}
		}

		return invariants::render().shadow_height;
	}();

	if (owner) {
		if (const auto rigid_body = owner.template find<components::rigid_body>()) {
			const auto scaled = std::round(base_height * rigid_body->get_raw_component().special.shadow_height_mult);
			return static_cast<uint8_t>(std::clamp(scaled, 0.0f, 255.0f));
		}
	}

	return base_height;
}

inline bool calc_fixture_reaches_ceiling(const cosmos& cosm, const b2Fixture& fix) {
	if (const auto owner = cosm[fix.GetUserData()]) {
		if (const auto render = owner.template find<invariants::render>()) {
			return render->reaches_ceiling;
		}
	}

	return false;
}

inline transformr calc_physical_body_transform(const b2Body& body, const si_scaling si) {
	const auto& b2_transform = body.GetTransform();
	return transformr(vec2(b2_transform.p), b2_transform.q.GetAngle()).to_user_space(si);
}

/*
	Circles are approximated with num_circle_sides.
*/

inline void gather_fixture_world_points(
	std::vector<vec2>& out,
	const b2Fixture& fix,
	const transformr body_transform,
	const si_scaling si,
	const int num_circle_sides
) {
	out.clear();

	auto to_world = [&](const b2Vec2 local_meters) {
		return body_transform.pos + vec2(si.get_pixels(vec2(local_meters))).rotate(body_transform.rotation);
	};

	const auto* const shape = fix.GetShape();

	if (shape->GetType() == b2Shape::e_polygon) {
		const auto& poly = static_cast<const b2PolygonShape&>(*shape);

		for (int v = 0; v < poly.GetVertexCount(); ++v) {
			out.push_back(to_world(poly.GetVertex(v)));
		}
	}
	else if (shape->GetType() == b2Shape::e_circle) {
		const auto& circle = static_cast<const b2CircleShape&>(*shape);

		for (int v = 0; v < num_circle_sides; ++v) {
			const auto offset = vec2::from_degrees(360.0f * v / num_circle_sides) * circle.m_radius;
			out.push_back(to_world(circle.m_p + b2Vec2(offset.x, offset.y)));
		}
	}
}
