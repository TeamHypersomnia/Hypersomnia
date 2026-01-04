#pragma once
#include "game/common_state/cosmos_navmesh.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"
#include "game/components/marker_component.h"
#include "game/enums/filters.h"
#include "game/cosmos/cosmos_solvable_inferred.h"

#include "3rdparty/Box2D/Dynamics/b2World.h"
#include "3rdparty/Box2D/Dynamics/b2Body.h"
#include "3rdparty/Box2D/Dynamics/b2Fixture.h"
#include "3rdparty/Box2D/Collision/b2Collision.h"
#include "3rdparty/Box2D/Collision/Shapes/b2PolygonShape.h"

/*
	Collects navmesh island bounds from the cosmos.
	First checks for NAV_ISLAND area markers, then falls back to 
	computing the AABB of all fixtures that hit the pathfinding filter.
*/

inline void collect_navmesh_island_bounds(
	const cosmos& cosm,
	std::vector<ltrbi>& island_bounds,
	const int cell_size
) {
	island_bounds.clear();

	/*
		Check for NAV_ISLAND area markers by iterating actual entities.
	*/

	cosm.for_each_having<invariants::area_marker>(
		[&](const auto& typed_handle) {
			const auto& marker = typed_handle.template get<invariants::area_marker>();

			if (marker.type == area_marker_type::NAV_ISLAND) {
				if (const auto aabb = typed_handle.find_aabb()) {
					island_bounds.push_back(ltrbi(
						static_cast<int>(aabb->l),
						static_cast<int>(aabb->t),
						static_cast<int>(aabb->r),
						static_cast<int>(aabb->b)
					));
				}
			}
		}
	);

	if (!island_bounds.empty()) {
		return;
	}

	/*
		Fall back: compute AABB from all fixtures that hit the pathfinding filter.
	*/

	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto& b2w = physics.get_b2world();
	const auto pathfinding_filter = predefined_queries::pathfinding();
	const auto si = cosm.get_si();

	ltrbi total_aabb;
	bool first = true;

	for (const b2Body* body = b2w.GetBodyList(); body != nullptr; body = body->GetNext()) {
		for (const b2Fixture* fixture = body->GetFixtureList(); fixture != nullptr; fixture = fixture->GetNext()) {
			const auto& filter = fixture->GetFilterData();

			/* Check if this fixture matches the pathfinding filter */
			const bool should_collide = 
				(filter.categoryBits & pathfinding_filter.maskBits) != 0 &&
				(pathfinding_filter.categoryBits & filter.maskBits) != 0;

			if (!should_collide) {
				continue;
			}

			const auto* shape = fixture->GetShape();
			const auto child_count = shape->GetChildCount();

			for (int child_idx = 0; child_idx < child_count; ++child_idx) {
				const auto& fixture_aabb = fixture->GetAABB(child_idx);

				/* Convert from meters to pixels */
				const auto l = static_cast<int>(si.get_pixels(fixture_aabb.lowerBound.x));
				const auto t = static_cast<int>(si.get_pixels(fixture_aabb.lowerBound.y));
				const auto r = static_cast<int>(si.get_pixels(fixture_aabb.upperBound.x));
				const auto b = static_cast<int>(si.get_pixels(fixture_aabb.upperBound.y));

				if (first) {
					total_aabb = ltrbi(l, t, r, b);
					first = false;
				}
				else {
					if (l < total_aabb.l) { total_aabb.l = l; }
					if (t < total_aabb.t) { total_aabb.t = t; }
					if (r > total_aabb.r) { total_aabb.r = r; }
					if (b > total_aabb.b) { total_aabb.b = b; }
				}
			}
		}
	}

	if (!first) {
		/* Expand the bounding box a bit for navigation */
		const auto margin = cell_size * 2;
		total_aabb.l -= margin;
		total_aabb.t -= margin;
		total_aabb.r += margin;
		total_aabb.b += margin;

		island_bounds.push_back(total_aabb);
	}
}

/*
	Rebuilds the occupied grid for this island by testing each cell 
	against fixtures in the physics world that match the pathfinding filter.
*/

inline void rebuild_navmesh_island_occupied(
	cosmos_navmesh_island& island,
	const cosmos& cosm
) {
	if (island.cell_size <= 0) {
		return;
	}

	island.resize_to_bounds();

	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto& b2w = physics.get_b2world();
	const auto pathfinding_filter = predefined_queries::pathfinding();
	const auto si = cosm.get_si();

	const auto cell_size = island.cell_size;
	const auto& aligned = island.bound;

	/*
		Pre-create the cell shape for collision testing.
		We reuse this for all cells, just updating the transform.
	*/

	b2PolygonShape cell_shape;
	const auto half_cell_meters = si.get_meters(static_cast<float>(cell_size) / 2.0f);
	cell_shape.SetAsBox(half_cell_meters, half_cell_meters);

	/*
		Iterate all fixtures in the b2World and mark occupied cells.
	*/

	for (const b2Body* body = b2w.GetBodyList(); body != nullptr; body = body->GetNext()) {
		for (const b2Fixture* fixture = body->GetFixtureList(); fixture != nullptr; fixture = fixture->GetNext()) {
			const auto& filter = fixture->GetFilterData();

			/* Check if this fixture matches the pathfinding filter */
			const bool should_collide = 
				(filter.categoryBits & pathfinding_filter.maskBits) != 0 &&
				(pathfinding_filter.categoryBits & filter.maskBits) != 0;

			if (!should_collide) {
				continue;
			}

			const auto* shape = fixture->GetShape();
			const auto child_count = shape->GetChildCount();
			const auto& body_xf = body->GetTransform();

			for (int child_idx = 0; child_idx < child_count; ++child_idx) {
				const auto& fixture_aabb = fixture->GetAABB(child_idx);

				/* Convert from meters to pixels */
				const auto lower_px = vec2(
					si.get_pixels(fixture_aabb.lowerBound.x),
					si.get_pixels(fixture_aabb.lowerBound.y)
				);
				const auto upper_px = vec2(
					si.get_pixels(fixture_aabb.upperBound.x),
					si.get_pixels(fixture_aabb.upperBound.y)
				);

				/* Align to grid (expand outward) */
				const auto grid_l = static_cast<int>(std::floor(lower_px.x / cell_size)) * cell_size;
				const auto grid_t = static_cast<int>(std::floor(lower_px.y / cell_size)) * cell_size;
				const auto grid_r = static_cast<int>(std::ceil(upper_px.x / cell_size)) * cell_size;
				const auto grid_b = static_cast<int>(std::ceil(upper_px.y / cell_size)) * cell_size;

				/* Iterate each cell in the grid-aligned AABB */
				for (int cy = grid_t; cy < grid_b; cy += cell_size) {
					for (int cx = grid_l; cx < grid_r; cx += cell_size) {
						/* Check if this cell is within the island bounds */
						if (cx < aligned.l || cx >= aligned.r || cy < aligned.t || cy >= aligned.b) {
							continue;
						}

						/*
							Test overlap with the fixture using the pre-created cell shape.
						*/

						const auto cell_center_x = cx + cell_size / 2.0f;
						const auto cell_center_y = cy + cell_size / 2.0f;

						b2Transform cell_xf;
						cell_xf.Set(
							b2Vec2(si.get_meters(cell_center_x), si.get_meters(cell_center_y)),
							0.0f
						);

						const auto overlaps = b2TestOverlap(
							shape, child_idx,
							&cell_shape, 0,
							body_xf,
							cell_xf
						);

						if (overlaps) {
							island.set(cx, cy, 1);
						}
					}
				}
			}
		}
	}
}

/*
	Generates a complete navmesh from the cosmos.
	1. Collects island bounds from NAV_ISLAND markers or fixture AABBs
	2. Creates islands with aligned bounds
	3. Rebuilds occupied cells for each island
*/

inline cosmos_navmesh generate_navmesh(
	const cosmos& cosm,
	const int cell_size
) {
	cosmos_navmesh navmesh;

	std::vector<ltrbi> island_bounds;
	collect_navmesh_island_bounds(cosm, island_bounds, cell_size);

	for (const auto& bounds : island_bounds) {
		/* Align bounds to cell_size grid */
		auto aligned = bounds;
		aligned.l = (aligned.l / cell_size) * cell_size;
		aligned.t = (aligned.t / cell_size) * cell_size;
		aligned.r = ((aligned.r + cell_size - 1) / cell_size) * cell_size;
		aligned.b = ((aligned.b + cell_size - 1) / cell_size) * cell_size;

		cosmos_navmesh_island island;
		island.bound = aligned;
		island.cell_size = cell_size;

		rebuild_navmesh_island_occupied(island, cosm);

		navmesh.islands.push_back(std::move(island));
	}

	return navmesh;
}
