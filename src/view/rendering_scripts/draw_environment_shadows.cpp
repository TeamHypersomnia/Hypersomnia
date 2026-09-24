#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

#include "augs/math/convex_hull.h"
#include "augs/math/vec2.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/enums/filters.h"
#include "game/components/render_component.h"
#include "game/components/sprite_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/marker_component.h"
#include "game/detail/visible_entities.hpp"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/detail/physics/physics_queries.h"

#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/rendering_scripts/draw_environment_shadows.h"

void draw_environment_shadows(const draw_environment_shadows_input in) {
	const auto& cosm = in.cosm;
	const auto& light = cosm.get_common_significant().light;
	const auto step = light.shadow_step;
	const auto si = cosm.get_si();
	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto blank_uv = in.blank_tex.get_center();

	/*
		Casters outside the view still throw shadows into it.
		They lie at most the longest possible shadow away, towards the sun.
	*/

	const auto longest_shadow = step * static_cast<float>(std::numeric_limits<uint8_t>::max());

	const auto query = [&]() {
		auto q = in.queried_camera_aabb;

		if (longest_shadow.x > 0.0f) {
			q.l -= longest_shadow.x;
		}
		else {
			q.r -= longest_shadow.x;
		}

		if (longest_shadow.y > 0.0f) {
			q.t -= longest_shadow.y;
		}
		else {
			q.b -= longest_shadow.y;
		}

		return q;
	}();

	auto bodies_query = b2Filter();
	bodies_query.categoryBits = 1 << int(filter_category::QUERY);

	bodies_query.maskBits =
		(1 << int(filter_category::WALL)) |
		(1 << int(filter_category::GLASS_OBSTACLE))
	;

	struct shadow_cast {
		uint8_t height = 0;
		uint8_t strength = 0;
		std::size_t first = 0;
		std::size_t count = 0;
	};

	auto casts = std::vector<shadow_cast>();
	auto cast_points = std::vector<vec2>();
	auto fixture_points = std::vector<vec2>();

	auto push_fan = [&](
		augs::vertex_triangle_buffer& buf,
		const vec2* const points,
		const std::size_t count,
		const rgba col
	) {
		for (std::size_t i = 2; i < count; ++i) {
			augs::vertex_triangle tri;

			tri.vertices[0].pos = points[0];
			tri.vertices[1].pos = points[i - 1];
			tri.vertices[2].pos = points[i];

			for (auto& v : tri.vertices) {
				v.color = col;
				v.texcoord = blank_uv;
			}

			buf.push_back(tri);
		}
	};

	auto gather_fixture_points = [&](const b2Fixture& fix, const transformr body_transform) {
		fixture_points.clear();

		auto to_world = [&](const b2Vec2 local_meters) {
			return body_transform.pos + vec2(si.get_pixels(vec2(local_meters))).rotate(body_transform.rotation);
		};

		const auto* const shape = fix.GetShape();

		if (shape->GetType() == b2Shape::e_polygon) {
			const auto& poly = static_cast<const b2PolygonShape&>(*shape);

			for (int v = 0; v < poly.GetVertexCount(); ++v) {
				fixture_points.push_back(to_world(poly.GetVertex(v)));
			}
		}
		else if (shape->GetType() == b2Shape::e_circle) {
			const auto& circle = static_cast<const b2CircleShape&>(*shape);
			const auto num_sides = 12;

			for (int v = 0; v < num_sides; ++v) {
				const auto offset = vec2::from_degrees(360.0f * v / num_sides) * circle.m_radius;
				fixture_points.push_back(to_world(circle.m_p + b2Vec2(offset.x, offset.y)));
			}
		}
	};

	physics.for_each_in_aabb(
		si,
		query.left_top(),
		query.right_bottom(),
		bodies_query,
		[&](const b2Fixture& fix) {
			if (fix.IsSensor()) {
				return callback_result::CONTINUE;
			}

			const auto* const body = fix.GetBody();

			const auto body_transform = [&]() {
				/*
					Interpolated so that the shadows of moving bodies don't jitter against their sprites.
				*/

				if (const auto body_owner = cosm[body->GetUserData()]) {
					if (const auto viewed = body_owner.find_viewing_transform(in.interp)) {
						return *viewed;
					}
				}

				const auto& b2_transform = body->GetTransform();
				return transformr(vec2(b2_transform.p), b2_transform.q.GetAngle()).to_user_space(si);
			}();

			gather_fixture_points(fix, body_transform);

			if (fixture_points.size() < 3) {
				return callback_result::CONTINUE;
			}

			const auto owner = cosm[fix.GetUserData()];

			const auto height = [&]() -> uint8_t {
				const auto base_height = [&]() {
					if (owner) {
						if (const auto render = owner.find<invariants::render>()) {
							return render->shadow_height;
						}
					}

					return invariants::render().shadow_height;
				}();

				if (owner) {
					if (const auto rigid_body = owner.find<components::rigid_body>()) {
						const auto scaled = std::round(base_height * rigid_body->get_raw_component().special.shadow_height_mult);
						return static_cast<uint8_t>(std::clamp(scaled, 0.0f, 255.0f));
					}
				}

				return base_height;
			}();

			push_fan(in.footprints_output, fixture_points.data(), fixture_points.size(), rgba(0, 0, height, 0));

			const auto filter = fix.GetFilterData();
			const bool blocks_bullets = (filter.maskBits & (1 << int(filter_category::FLYING_BULLET))) != 0;

			if (!blocks_bullets || height == 0) {
				return callback_result::CONTINUE;
			}

			/*
				Only the resource's alpha - nodes often fade their color for looks,
				which shouldn't weaken the shadow.
			*/

			const auto strength = [&]() -> uint8_t {
				if (owner) {
					if (const auto sprite = owner.find<invariants::sprite>()) {
						return sprite->color.a;
					}
				}

				return 255;
			}();

			const auto offset = step * static_cast<float>(height);
			const auto num_points = fixture_points.size();

			for (std::size_t i = 0; i < num_points; ++i) {
				fixture_points.push_back(fixture_points[i] + offset);
			}

			const auto extruded = augs::convex_hull(fixture_points);

			casts.push_back({ height, strength, cast_points.size(), extruded.size() });
			cast_points.insert(cast_points.end(), extruded.begin(), extruded.end());

			return callback_result::CONTINUE;
		}
	);

	std::sort(
		casts.begin(),
		casts.end(),
		[](const shadow_cast& a, const shadow_cast& b) {
			if (a.height != b.height) {
				return a.height < b.height;
			}

			return a.strength < b.strength;
		}
	);

	for (const auto& c : casts) {
		push_fan(in.casts_output, cast_points.data() + c.first, c.count, rgba(c.height, c.strength, 0, 0));
	}

	in.visible.for_each<render_layer::AREA_MARKERS>(cosm, [&](const auto& handle) {
		handle.template dispatch_on_having_all<invariants::area_marker>([&](const auto& typed_handle) {
			if (typed_handle.template get<invariants::area_marker>().type != area_marker_type::NO_SHADOW) {
				return;
			}

			const auto where = typed_handle.get_logic_transform();
			const auto size = typed_handle.get_logical_size();

			const auto shape = [&]() {
				if (const auto marker = typed_handle.template find<components::marker>()) {
					return marker->shape;
				}

				return marker_shape_type::BOX;
			}();

			fixture_points.clear();

			if (shape == marker_shape_type::CIRCLE) {
				const auto radius = std::max(size.x, size.y) * 0.5f;
				const auto num_sides = 32;

				for (int v = 0; v < num_sides; ++v) {
					fixture_points.push_back(where.pos + vec2::from_degrees(360.0f * v / num_sides) * radius);
				}
			}
			else {
				const auto half = size / 2;

				for (const auto corner : { vec2(-half.x, -half.y), vec2(half.x, -half.y), vec2(half.x, half.y), vec2(-half.x, half.y) }) {
					fixture_points.push_back(where.pos + vec2(corner).rotate(where.rotation));
				}
			}

			push_fan(in.footprints_output, fixture_points.data(), fixture_points.size(), rgba(0, 0, 0, 255));
		});
	});
}
