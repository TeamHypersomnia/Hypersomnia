#pragma once
#include "game/cosmos/entity_handle.h"

template <class E, class F>
decltype(auto) for_each_fixture(const E& handle, F callback) {
	const auto& physics = handle.get_cosmos().get_solvable_inferred().physics;

	if (const auto ch = physics.find_colliders_cache(handle.get_id())) {
		for (const auto& f : ch->constructed_fixtures) {
			decltype(auto) result = callback(*f);

			if (result != std::nullopt) {
				return result;
			}
		}
	}

	return std::nullopt;
}

inline auto shape_overlaps_fixture(
	const b2Shape* const shape,
	const si_scaling si,
	const vec2 shape_pos,
	const b2Fixture& fixture
) -> std::optional<b2TestOverlapOutput> {
	const auto queried_shape_transform = b2Transform(b2Vec2(si.get_meters(shape_pos)), b2Rot(0.f));

	constexpr auto index_a = 0;
	constexpr auto index_b = 0;

	if (const auto result = b2TestOverlapInfo(
		shape,
		index_a,
		fixture.GetShape(),
		index_b,
		queried_shape_transform,
		fixture.GetBody()->GetTransform()
	); result.overlap) {
		return result;
	}

	return std::nullopt;
}

template <class E>
auto shape_overlaps_entity(
	const b2Shape* const shape,
	const vec2 shape_pos,
	const E& handle
) {
	const auto si = handle.get_cosmos().get_si();
	const auto queried_shape_transform = b2Transform(b2Vec2(si.get_meters(shape_pos)), b2Rot(0.f));

	return for_each_fixture(handle, [&](const b2Fixture& fixture) -> std::optional<b2TestOverlapOutput> {
		constexpr auto index_a = 0;
		constexpr auto index_b = 0;

		if (const auto result = b2TestOverlapInfo(
			shape,
			index_a,
			fixture.GetShape(),
			index_b,
			queried_shape_transform,
			fixture.GetBody()->GetTransform()
		); result.overlap) {
			return result;
		}

		return std::nullopt;
	});
}

