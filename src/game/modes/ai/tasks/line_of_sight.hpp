#pragma once
#include <Box2D/Collision/Shapes/b2PolygonShape.h>
#include "game/cosmos/cosmos.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/inferred_caches/find_physics_cache.h"
#include "game/enums/filters.h"

/*
	Check if a position is within line of sight from another position.
	Uses the physics system for raycasting.
*/

template <class E>
inline bool is_in_line_of_sight(
	const vec2 from_pos,
	const vec2 to_pos,
	const physics_world_cache& physics,
	const cosmos& cosm,
	const E& ignore_entity
) {
	const auto filter = predefined_queries::line_of_sight();
	const auto raycast = physics.ray_cast_px(
		cosm.get_si(),
		from_pos,
		to_pos,
		filter,
		ignore_entity
	);

	return !raycast.hit;
}

/*
	Returns true if ANY polygon vertex of entity_handle has a clear line of sight
	from source_pos (i.e., the raycast hits nothing between the source and the vertx).
	Falls back to the entity's logical position when no polygon fixture is found.
*/
template <class E, class Physics>
bool los_to_any_vertices_of(
	const E& entity_handle,
	const vec2 source_pos,
	const Physics& physics,
	const si_scaling si,
	const b2Filter filter
) {
	if (const auto* cc = ::find_colliders_cache(entity_handle)) {
		for (const auto& fp : cc->constructed_fixtures) {
			const auto* f = fp.get();
			const auto* shape = f->GetShape();

			if (shape->GetType() != b2Shape::e_polygon) {
				continue;
			}

			const auto& poly = static_cast<const b2PolygonShape&>(*shape);
			const auto& xf = f->GetBody()->GetTransform();

			for (int v = 0; v < poly.GetVertexCount(); ++v) {
				const auto world_px = si.get_pixels(static_cast<vec2>(b2Mul(xf, poly.GetVertex(v))));

				if (!physics.ray_cast_px(si, source_pos, world_px, filter).hit) {
					return true;
				}
			}
		}

		return false;
	}

	/* No fixture data — fall back to entity centre */
	if (const auto transform = entity_handle.find_logic_transform()) {
		return !physics.ray_cast_px(si, source_pos, transform->pos, filter).hit;
	}

	return false;
}

/*
	For each polygon vertex of entity_handle, raycasts in the given unit
	direction up to length pixels using the entity's own body as the ignore
	target.  Returns the sum of free distances across all vertices
	(distance to first wall, or the full length when unobstructed).
	Falls back to the entity's logical position when no polygon fixture is found.
*/
template <class E, class Physics>
float sense_free_space_from_vertices_of(
	const E& entity_handle,
	const vec2 direction,
	const float length,
	const Physics& physics,
	const si_scaling si,
	const b2Filter filter
) {
	float total_free = 0.0f;
	int num_rays = 0;

	auto cast_one = [&](const vec2 from) {
		const auto rc = physics.ray_cast_px(
			si,
			from,
			from + direction * length,
			filter,
			entity_handle
		);

		total_free += rc.hit ? (rc.intersection - from).length() : length;
		++num_rays;
	};

	if (const auto* cc = ::find_colliders_cache(entity_handle)) {
		for (const auto& fp : cc->constructed_fixtures) {
			const auto* f = fp.get();
			const auto* shape = f->GetShape();

			if (shape->GetType() != b2Shape::e_polygon) {
				continue;
			}

			const auto& poly = static_cast<const b2PolygonShape&>(*shape);
			const auto& xf = f->GetBody()->GetTransform();

			for (int v = 0; v < poly.GetVertexCount(); ++v) {
				cast_one(si.get_pixels(static_cast<vec2>(b2Mul(xf, poly.GetVertex(v)))));
			}
		}
	}

	if (num_rays == 0) {
		/* No polygon fixtures — fall back to entity centre */
		if (const auto transform = entity_handle.find_logic_transform()) {
			cast_one(transform->pos);
		}
	}

	return total_free;
}

