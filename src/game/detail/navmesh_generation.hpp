#pragma once
#include "game/common_state/cosmos_navmesh.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"
#include "game/components/marker_component.h"
#include "game/components/portal_component.h"
#include "game/enums/filters.h"
#include "game/cosmos/cosmos_solvable_inferred.h"

#include "3rdparty/Box2D/Dynamics/b2World.h"
#include "3rdparty/Box2D/Dynamics/b2Body.h"
#include "3rdparty/Box2D/Dynamics/b2Fixture.h"
#include "3rdparty/Box2D/Collision/b2Collision.h"
#include "3rdparty/Box2D/Collision/Shapes/b2PolygonShape.h"

/*
	We make aabb's of the fixtures a tiny bit thinner,
	so they don't bleed out into neighboring grid cells.

	This is required even though we compute AABBs directly:
	2 pixels are necessary since Box2D also adds
	a radius of a pixel to all polygons. 
*/

constexpr int fattening_epsilon_v = -2;

/*
	Maximum number of portals per island.
	Cell values: 0 = free, 1 = occupied, 2-255 = portal index + 2.
	So we can have 255 - 2 = 253 portals per island.
*/
constexpr std::size_t max_portals_per_island_v = 253;

/*
	Computes exact AABB from the shape, avoiding the potentially fattened proxy AABB.
*/

inline b2AABB compute_exact_fixture_aabb(
	const b2Fixture* fixture,
	const b2Transform& body_xf,
	const int child_idx
) {
	b2AABB aabb;
	fixture->GetShape()->ComputeAABB(&aabb, body_xf, child_idx);
	return aabb;
}

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
		const auto& body_xf = body->GetTransform();

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
				/*
					Compute exact AABB from the shape instead of using the proxy AABB
					which might be fattened.
				*/
				const auto fixture_aabb = ::compute_exact_fixture_aabb(fixture, body_xf, child_idx);

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
	Fills the navmesh grid for a given fixture AABB with the specified value.
	Can use an optimized path for axis-aligned boxes when applicable.
*/

inline void fill_navmesh_grid_from_fixture(
	cosmos_navmesh_island& island,
	const b2Fixture* fixture,
	const si_scaling& si,
	b2PolygonShape& cell_shape,
	const uint8_t fill_value
) {
	const auto cell_size = island.cell_size;
	const auto& aligned = island.bound;
	const auto* body = fixture->GetBody();
	const auto& body_xf = body->GetTransform();
	const auto* shape = fixture->GetShape();
	const auto child_count = shape->GetChildCount();

	for (int child_idx = 0; child_idx < child_count; ++child_idx) {
		/*
			Compute exact AABB from the shape instead of using the proxy AABB.
		*/
		const auto fixture_aabb = ::compute_exact_fixture_aabb(fixture, body_xf, child_idx);

		/* Convert from meters to pixels */
		auto lower_px = vec2(
			si.get_pixels(fixture_aabb.lowerBound.x) - fattening_epsilon_v,
			si.get_pixels(fixture_aabb.lowerBound.y) - fattening_epsilon_v
		);

		auto upper_px = vec2(
			si.get_pixels(fixture_aabb.upperBound.x) + fattening_epsilon_v,
			si.get_pixels(fixture_aabb.upperBound.y) + fattening_epsilon_v
		);

		/*
			Don't overcompensate, just in case
		*/
		if (lower_px.x > upper_px.x) {
			lower_px.x = upper_px.x = (lower_px.x + upper_px.x) / 2;
			lower_px.x -= 1;
			upper_px.x += 1;
		}

		if (lower_px.y > upper_px.y) {
			lower_px.y = upper_px.y = (lower_px.y + upper_px.y) / 2;
			lower_px.y -= 1;
			upper_px.y += 1;
		}

		/*
			Early out if the fixture's AABB doesn't overlap with the island bounds.
		*/
		const bool outside_island =
			upper_px.x < aligned.l ||
			lower_px.x > aligned.r ||
			upper_px.y < aligned.t ||
			lower_px.y > aligned.b
		;

		if (outside_island) {
			continue;
		}

		/* Align to grid (expand outward) */
		const auto grid_l = static_cast<int>(std::floor(lower_px.x / cell_size)) * cell_size;
		const auto grid_t = static_cast<int>(std::floor(lower_px.y / cell_size)) * cell_size;
		const auto grid_r = static_cast<int>(std::ceil(upper_px.x / cell_size)) * cell_size;
		const auto grid_b = static_cast<int>(std::ceil(upper_px.y / cell_size)) * cell_size;

		/*
			Check if we can use the optimized AABB path:
			- The shape must be a polygon with m_is_aabb flag set
			- The body's rotation must be approximately 0
		*/
		const bool body_has_no_rotation = body_xf.q.IsNearZero();

		bool use_aabb_optimization = false;

		if (body_has_no_rotation && shape->GetType() == b2Shape::e_polygon) {
			const auto* poly = static_cast<const b2PolygonShape*>(shape);
			use_aabb_optimization = poly->m_is_aabb;
		}

		if (use_aabb_optimization) {
			/*
				Optimized path: simply fill the grid cells covered by the AABB
				without per-cell collision testing.
				Iterate in y, x order for cache-friendliness.
			*/
			for (int cy = grid_t; cy < grid_b; cy += cell_size) {
				for (int cx = grid_l; cx < grid_r; cx += cell_size) {
					if (cx < aligned.l || cx >= aligned.r || cy < aligned.t || cy >= aligned.b) {
						continue;
					}

					island.set(cx, cy, fill_value);
				}
			}
		}
		else {
			/*
				Standard path: test each cell for overlap with the fixture.
			*/
			for (int cy = grid_t; cy < grid_b; cy += cell_size) {
				for (int cx = grid_l; cx < grid_r; cx += cell_size) {
					if (cx < aligned.l || cx >= aligned.r || cy < aligned.t || cy >= aligned.b) {
						continue;
					}

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
						island.set(cx, cy, fill_value);
					}
				}
			}
		}
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

			::fill_navmesh_grid_from_fixture(island, fixture, si, cell_shape, 1);
		}
	}
}

/*
	Finds which island index contains the given world position.
	Returns -1 if the position is not within any island bounds.
*/

inline int find_island_index_for_position(
	const cosmos_navmesh& navmesh,
	const vec2 world_pos
) {
	const auto pos_i = vec2i(static_cast<int>(world_pos.x), static_cast<int>(world_pos.y));

	for (std::size_t i = 0; i < navmesh.islands.size(); ++i) {
		const auto& island = navmesh.islands[i];
		const auto& bound = island.bound;

		if (pos_i.x >= bound.l && pos_i.x < bound.r &&
			pos_i.y >= bound.t && pos_i.y < bound.b) {
			return static_cast<int>(i);
		}
	}

	return -1;
}

/*
	Processes portals and adds them to the navmesh.
	- Portals with hazard field enabled are marked as occupied (value 1)
	- Portals with valid exits are marked with special values (2 + portal_index)
*/

inline void process_portals_for_navmesh(
	cosmos_navmesh& navmesh,
	const cosmos& cosm
) {
	if (navmesh.islands.empty()) {
		return;
	}

	const auto si = cosm.get_si();

	/*
		Pre-create cell shapes for each island for collision testing.
		We need a vector because islands are of potentially different sizes.
	*/
	std::vector<b2PolygonShape> cell_shapes;
	cell_shapes.reserve(navmesh.islands.size());

	for (auto& island : navmesh.islands) {
		b2PolygonShape cell_shape;
		const auto half_cell_meters = si.get_meters(static_cast<float>(island.cell_size) / 2.0f);
		cell_shape.SetAsBox(half_cell_meters, half_cell_meters);
		cell_shapes.push_back(cell_shape);
	}

	using C = filter_category;

	cosm.for_each_having<components::portal>(
		[&](const auto& typed_portal_handle) {
			const auto& portal = typed_portal_handle.template get<components::portal>();

			/*
				Only consider portals that collide with CHARACTER or CHARACTER_WEAPON.
			*/
			const auto& filter = portal.custom_filter;
			const bool interacts_with_characters =
				(filter.maskBits & (1 << static_cast<int>(C::CHARACTER))) != 0 ||
				(filter.maskBits & (1 << static_cast<int>(C::CHARACTER_WEAPON))) != 0
			;

			if (!interacts_with_characters) {
				return;
			}

			/*
				Find which island this portal belongs to based on its position.
			*/
			const auto portal_pos = typed_portal_handle.get_logic_transform().pos;
			const auto island_idx = ::find_island_index_for_position(navmesh, portal_pos);

			if (island_idx < 0) {
				return;
			}

			auto& island = navmesh.islands[island_idx];

			/*
				Helper to fill grid cells for all fixtures of this portal.
			*/
			auto fill_portal_fixtures = [&](const uint8_t fill_value) {
				if (const auto rigid_body = typed_portal_handle.template find<components::rigid_body>()) {
					if (const auto b2body = rigid_body.find_body()) {
						for (const b2Fixture* fixture = b2body->GetFixtureList(); fixture != nullptr; fixture = fixture->GetNext()) {
							::fill_navmesh_grid_from_fixture(
								island,
								fixture,
								si,
								cell_shapes[island_idx],
								fill_value
							);
						}
					}
				}
			};

			const bool has_hazard = portal.hazard.is_enabled;
			const bool has_valid_exit = portal.portal_exit.is_set() && cosm[portal.portal_exit].alive();

			/*
				If hazard field is enabled, mark as occupied regardless of exit.
			*/
			if (has_hazard) {
				fill_portal_fixtures(1);
				return;
			}

			/*
				Process portals with valid exits.
			*/
			if (!has_valid_exit) {
				return;
			}

			const auto portal_index = island.portals.size();

			if (portal_index >= max_portals_per_island_v) {
				return;
			}

			const auto fill_value = static_cast<uint8_t>(2 + portal_index);

			/*
				Find exit portal position and island.
			*/
			const auto exit_handle = cosm[portal.portal_exit];
			const auto exit_pos = exit_handle.get_logic_transform().pos;
			const auto exit_island_idx = ::find_island_index_for_position(navmesh, exit_pos);

			/*
				Compute exit cell position in the target island.
			*/
			auto out_cell_pos = vec2i::zero;

			if (exit_island_idx >= 0) {
				const auto& exit_island = navmesh.islands[exit_island_idx];
				const auto exit_pos_i = vec2i(exit_pos);
				const auto bound_lt = vec2i(exit_island.bound.l, exit_island.bound.t);

				out_cell_pos = (exit_pos_i - bound_lt) / exit_island.cell_size;
			}

			/*
				Create portal entry.
			*/
			navmesh_portal portal_entry;
			portal_entry.out_cell_pos = out_cell_pos;
			portal_entry.out_island_index = exit_island_idx;
			island.portals.push_back(portal_entry);

			/*
				Fill the grid cells covered by this portal's fixtures.
			*/
			fill_portal_fixtures(fill_value);
		}
	);
}

/*
	Generates a complete navmesh from the cosmos.
	1. Collects island bounds from NAV_ISLAND markers or fixture AABBs
	2. Creates islands with aligned bounds
	3. Rebuilds occupied cells for each island
	4. Processes portals and adds them to the navmesh
*/

inline cosmos_navmesh generate_navmesh(
	const cosmos& cosm,
	const int cell_size
) {
	cosmos_navmesh navmesh;

	std::vector<ltrbi> island_bounds;
	::collect_navmesh_island_bounds(cosm, island_bounds, cell_size);

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

	/*
		Process portals as the last step of navmesh generation.
	*/
	process_portals_for_navmesh(navmesh, cosm);

	return navmesh;
}
