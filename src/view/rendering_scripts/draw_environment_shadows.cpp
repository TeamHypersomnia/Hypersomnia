#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

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
#include "view/rendering_scripts/shadow_casters.h"

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

	auto fixture_points = std::vector<vec2>();

	auto push_triangle = [&](
		augs::vertex_triangle_buffer& buf,
		const std::array<vec2, 3> positions,
		const std::array<rgba, 3> colors
	) {
		augs::vertex_triangle tri;

		for (std::size_t i = 0; i < 3; ++i) {
			tri.vertices[i].pos = positions[i];
			tri.vertices[i].color = colors[i];
			tri.vertices[i].texcoord = blank_uv;
		}

		buf.push_back(tri);
	};

	auto push_fan = [&](
		augs::vertex_triangle_buffer& buf,
		const vec2* const points,
		const std::size_t count,
		const rgba col
	) {
		for (std::size_t i = 2; i < count; ++i) {
			push_triangle(buf, { points[0], points[i - 1], points[i] }, { col, col, col });
		}
	};

	/*
		The shadow of a convex fixture is the fixture itself
		plus one parallelogram swept along the shadow from every edge facing away from the sun.
		These tile the shadow exactly, and their colors interpolate linearly along its length.
	*/

	auto push_cast = [&](const vec2 offset, const rgba base_col, const rgba tip_col) {
		const auto num_points = fixture_points.size();

		push_fan(in.casts_output, fixture_points.data(), num_points, base_col);

		auto center = vec2::zero;

		for (const auto& p : fixture_points) {
			center += p;
		}

		center /= static_cast<float>(num_points);

		for (std::size_t i = 0; i < num_points; ++i) {
			const auto a = fixture_points[i];
			const auto b = fixture_points[(i + 1) % num_points];

			const auto outward = [&]() {
				const auto normal = (b - a).perpendicular_cw();
				return normal.dot(a - center) < 0.0f ? -normal : normal;
			}();

			if (outward.dot(offset) <= 0.0f) {
				continue;
			}

			push_triangle(in.casts_output, { a, b, b + offset }, { base_col, base_col, tip_col });
			push_triangle(in.casts_output, { a, b + offset, a + offset }, { base_col, tip_col, tip_col });
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

				return ::calc_physical_body_transform(*body, si);
			}();

			::gather_fixture_world_points(fixture_points, fix, body_transform, si, 12);

			if (fixture_points.size() < 3) {
				return callback_result::CONTINUE;
			}

			const auto owner = cosm[fix.GetUserData()];

			const auto height = ::calc_fixture_shadow_height(cosm, fix);

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

			const auto tip_strength = static_cast<uint8_t>(std::round(strength * in.tip_strength));

			push_cast(
				step * static_cast<float>(height),
				rgba(height, strength, 0, 0),
				rgba(height, tip_strength, 0, 0)
			);

			return callback_result::CONTINUE;
		}
	);

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
