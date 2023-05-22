#pragma once
#include "game/enums/marker_type.h"
#include "game/cosmos/for_each_entity.h"
#include "game/inferred_caches/is_grid_organism.h"

constexpr real32 movement_path_neighbor_query_radius_v = 50.f;
constexpr int grid_cell_size_v = static_cast<int>(movement_path_neighbor_query_radius_v * 2);

using organism_id_type = organism_cache::organism_id_type;
using cell_type = organism_cache::cell_type;

template <class E>
bool organism_cache::assign_to_grid(const E& organism) {
	if (!::is_grid_organism(organism)) {
		return false;
	}
	
	const auto& cosm = organism.get_cosmos();
	const auto& movement_path = organism.template get<components::movement_path>();
	const auto origin = cosm[movement_path.origin];

	if (!origin.alive()) {
		return false;
	}

	const auto grid_id = origin.get_id();
	auto& grid = grids[grid_id];

	if (!grid.is_set()) {
		if (!reset_grid_for(origin)) {
			return false;
		}
	}

	if (const auto t = organism.find_logic_transform()) {
		const auto p = t->pos;

		auto& cell = grid.get_cell_at_world(p);
		cell.organisms.emplace_back(organism.get_id());

		return true;
	}

	return false;
}

template <class E>
bool organism_cache::reset_grid_for(const E& area) {
	const auto id = area.get_id().to_unversioned();

#if 0
	LOG("Resetting grid for %x", area);
#endif

	if (const auto marker = area.template find<invariants::area_marker>()) {
		if (marker->type == area_marker_type::ORGANISM_AREA) {
			if (const auto aabb = area.find_aabb()) {
				auto& grid = grids[id];
				grid.clear();
				grid.reset(*aabb);
				return true;
			}
		}
	}

	grids.erase(id);
	return false;
}

template <class E>
void organism_cache::recalculate_grid(const E& area) {
	auto& cosm = area.get_cosmos();

	if (!reset_grid_for(area)) {
		return;
	}

	cosm.template for_each_entity<is_potential_organism>([&](const auto& organism) {
		const auto& movement_path = organism.template get<components::movement_path>();

		if (movement_path.origin == area.get_id()) {
			assign_to_grid(organism);
		}
	});
}

template <class E>
void organism_cache::specific_infer_cache_for(const E& handle) {
	if constexpr(E::template has<invariants::area_marker>()) {
		recalculate_grid(handle);
	}
	else {
		assign_to_grid(handle);
	}
}

inline bool organism_cache::recalculate_cell_for(
	const unversioned_entity_id origin,
	const organism_id_type organism_id,
   	const vec2 old_position,
   	const vec2 new_position
) {
	if (const auto grid = find_grid(origin)) {
		grid->erase_organism_from_position(organism_id, old_position);

		auto& new_orgs = grid->get_cell_at_world(new_position).organisms;
		new_orgs.emplace_back(organism_id);

		return true;
	}

	return false;
}

inline cell_type& organism_cache::grid::get_cell(const vec2i p) {
	return cells[cells_size().x * p.y + p.x];
}

inline const cell_type& organism_cache::grid::get_cell(const vec2i p) const {
	return cells[cells_size().x * p.y + p.x];
}

inline vec2i organism_cache::grid::get_cell_coord_at_world(const vec2 w) const {
	const auto local_snapped_x = static_cast<int>(std::max(0.f, w.x - aabb.l));
	const auto local_snapped_y = static_cast<int>(std::max(0.f, w.y - aabb.t));
	
	const auto sz = cells_size();

	const auto coord_x = std::min(sz.x - 1, local_snapped_x / grid_cell_size_v);
	const auto coord_y = std::min(sz.y - 1, local_snapped_y / grid_cell_size_v);

	return vec2i(coord_x, coord_y);
}

inline cell_type& organism_cache::grid::get_cell_at_world(const vec2 w) {
	return get_cell(get_cell_coord_at_world(w));
}

inline const cell_type& organism_cache::grid::get_cell_at_world(const vec2 w) const {
	return get_cell(get_cell_coord_at_world(w));
}

inline vec2i organism_cache::grid::cells_size() const {
	const auto cells_hori = 1 + static_cast<int>(aabb.w() / grid_cell_size_v);
	const auto cells_vert = 1 + static_cast<int>(aabb.h() / grid_cell_size_v);

	return { cells_hori, cells_vert };
}

inline bool organism_cache::grid::is_set() const {
	return aabb.good();
}

inline void organism_cache::grid::erase_organism_from_position(const organism_id_type id, const vec2 pos) {
	auto& old_orgs = get_cell_at_world(pos).organisms;

	const auto before_size = old_orgs.size();
	erase_element(old_orgs, id);

	if (old_orgs.size() == before_size) {
		erase_organism(id);
	}
}

inline organism_cache::grid* organism_cache::find_grid(const unversioned_entity_id id) {
	return mapped_or_nullptr(grids, id);
}

inline const organism_cache::grid* organism_cache::find_grid(const unversioned_entity_id id) const {
	return mapped_or_nullptr(grids, id);
}

