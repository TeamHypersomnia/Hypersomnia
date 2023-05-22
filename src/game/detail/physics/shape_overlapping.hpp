#pragma once
#include "game/cosmos/entity_handle.h"
#include "game/detail/physics/shape_helpers.h"
#include "augs/templates/traits/is_nullopt.h"
#include "game/inferred_caches/find_physics_cache.h"

template <class E, class F>
auto for_each_fixture(const E& handle, F callback) -> decltype(callback(std::declval<b2Fixture&>())) {
	if (const auto ch = find_colliders_cache(handle)) {
		for (const auto& f : ch->constructed_fixtures) {
			decltype(auto) result = callback(*f);

			if (result.has_value()) {
				return result;
			}
		}
	}

	return std::nullopt;
}

inline auto shape_overlaps_fixture(
	const b2Shape* const shape,
	const si_scaling si,
	const transformr shape_transform,
	const b2Fixture& fixture
) -> std::optional<b2TestOverlapOutput> {
	constexpr auto index_a = 0;
	constexpr auto index_b = 0;

	if (const auto result = b2TestOverlapInfo(
		shape,
		index_a,
		fixture.GetShape(),
		index_b,
		shape_transform.to<b2Transform>(si),
		fixture.GetBody()->GetTransform()
	); result.overlap) {
		return result;
	}

	return std::nullopt;
}

template <class E>
auto shape_overlaps_entity(
	const b2Shape* const shape,
	const transformr shape_transform,
	const E& handle
) {
	const auto si = handle.get_cosmos().get_si();

	return for_each_fixture(handle, [&](const b2Fixture& fixture) -> std::optional<b2TestOverlapOutput> {
		if (const auto result = shape_overlaps_fixture(
			shape,
			si,
			shape_transform,
			fixture
		); result.has_value() && result->overlap) {
			return result;
		}

		return std::nullopt;
	});
}

template <class A, class F>
auto on_shape_representation(
	const A& shapized_entity,
	F&& callback
) {
	if constexpr(A::template has<invariants::area_marker>()) {
		const auto& b = shapized_entity.get_logical_size(); 
		const auto si = shapized_entity.get_cosmos().get_si();

		b2PolygonShape shape;
		const auto hx = si.get_meters(b.x / 2);
		const auto hy = si.get_meters(b.y / 2);

		shape.SetAsBox(hx, hy);
		return callback(shape);
	}

	return callback(std::nullopt);
}

template <class A, class B>
auto entity_overlaps_entity(
	const A& shapized_entity,
	const B& queried_entity
) -> std::optional<b2TestOverlapOutput> {
	const auto shape_transform = shapized_entity.get_logic_transform();

	if (shapized_entity.template has<invariants::fixtures>()) {
		/* TODO */
		ensure(false && "not implemented");
	}
	else {
		return on_shape_representation(shapized_entity, [&](const auto& shape) -> std::optional<b2TestOverlapOutput> {
			if constexpr(is_nullopt_v<decltype(shape)>) {
				return std::nullopt;
			}
			else {
				return shape_overlaps_entity(std::addressof(shape), shape_transform, queried_entity);
			}
		});
	}

	return std::nullopt;
}

template <class E>
auto circular_sector_overlaps_entity(
	const transformr shape_transform,
	const real32 radius,
	const real32 sector_angle,
	const real32 sector_spread_degrees,
	const E& handle
) -> std::optional<b2TestOverlapOutput> {
	const auto si = handle.get_cosmos().get_si();

	{
		b2CircleShape shape;
		shape.m_radius = si.get_meters(radius);

		if (const auto result = shape_overlaps_entity(
			&shape,
			shape_transform,
			handle
		); result == std::nullopt) {
			return std::nullopt;
		}
	}

	const auto& rot = sector_angle;
	const auto& a = sector_spread_degrees;
	const auto& r = radius;

	const auto sector_dir = vec2::from_degrees(rot);
	const auto sector_tip = sector_dir * r;

	const auto tangent = vec2::segment_type {
		sector_tip,
		sector_tip + sector_tip.perpendicular_cw()
	};

	const auto left_edge_off = r * vec2::from_degrees(rot - a / 2);
	const auto right_edge_off = r * vec2::from_degrees(rot + a / 2);

	const auto over_left_edge = left_edge_off.closest_point_on_line(tangent);
	const auto over_right_edge = right_edge_off.closest_point_on_line(tangent);

	std::array<vec2, 5> v;

	v[0] = b2Vec2(0.f, 0.f);
	v[1] = b2Vec2(left_edge_off);
	v[2] = b2Vec2(over_left_edge);
	v[3] = b2Vec2(over_right_edge);
	v[4] = b2Vec2(right_edge_off);

#if 0
	debug_draw_verts(DEBUG_LOGIC_STEP_LINES, white, v, shape_pos);
	debug_draw_verts(DEBUG_LOGIC_STEP_LINES, cyan, tangent, shape_pos);
	debug_draw_verts(DEBUG_LOGIC_STEP_LINES, red, vec2::segment_type{ vec2::zero, sector_tip }, shape_pos);
#endif

	const auto shape = to_polygon_shape(v, si);

	if (const auto result = shape_overlaps_entity(
		&shape,
		shape_transform,
		handle
	); result.has_value() && result->overlap) {
		return result;
	}

	return std::nullopt;
}

template <class A>
vec2 get_interaction_query_center(
	const A& handle
) {
	const auto& sentience_def = handle.template get<invariants::sentience>();
	const auto tr = handle.get_logic_transform();

	return tr.pos + tr.get_direction() * sentience_def.interaction_hitbox_radius / 2;
}

template <class A>
vec2 get_interaction_query_top(
	const A& handle
) {
	const auto& sentience_def = handle.template get<invariants::sentience>();
	const auto tr = handle.get_logic_transform();

	return tr.pos + tr.get_direction() * sentience_def.interaction_hitbox_radius;
}

template <class A, class B>
auto interaction_hitbox_overlaps(
	const A& handle,
	const B& other
) -> std::optional<b2TestOverlapOutput> {
	const auto& sentience_def = handle.template get<invariants::sentience>();
	const auto tr = handle.get_logic_transform();

	if (const auto result = circular_sector_overlaps_entity(
		tr.pos,
		sentience_def.interaction_hitbox_radius,
		tr.rotation,
		sentience_def.interaction_hitbox_range,
		other
	)) {
		return result;
	}

	return std::nullopt;
}
