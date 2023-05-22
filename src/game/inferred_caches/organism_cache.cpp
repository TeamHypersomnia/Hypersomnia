#include "game/inferred_caches/organism_cache.h"
#include "augs/templates/container_templates.h"
#include "game/inferred_caches/organism_cache.hpp"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"

void organism_cache::grid::erase_organism(const organism_id_type id) {
	for (auto& cell : cells) {
		erase_element(cell.organisms, id);
	}
}

void organism_cache::erase_organism(const organism_id_type id) {
	for (auto& g : grids) {
		g.second.erase_organism(id);
	}
}

void organism_cache::grid::clear() {
	aabb = {};

	for (auto& v : cells) {
		v.organisms.clear();
	}
}

void organism_cache::grid::reset(const ltrb new_aabb) {
	clear();

	aabb = new_aabb;
	const auto num_cells = cells_size().area();
	cells.resize(num_cells);
}

void organism_cache::destroy_cache_of(const const_entity_handle& handle) {
	using concerned = entity_types_passing<concerned_with>;

	handle.constrained_dispatch<concerned>([&](const auto& typed_handle) {
		using E = remove_cref<decltype(typed_handle)>;

		if constexpr(E::template has<invariants::area_marker>()) {
			const auto id = handle.get_id();
			grids.erase(id);
		}
		else {
			const auto& movement_path = typed_handle.template get<components::movement_path>();
			const auto origin = movement_path.origin;

			if (const auto grid = find_grid(origin)) {
				grid->erase_organism_from_position(typed_handle.get_id(), typed_handle.get_logic_transform().pos);
			}
			else {
				erase_organism(typed_handle.get_id());
			}
		}
	});
}

void organism_cache::infer_all(const cosmos& cosm) {
	cosm.for_each_entity<is_potential_organism>([this](const auto& organism) {
		assign_to_grid(organism);
	});
}

void organism_cache::infer_cache_for(const const_entity_handle& e) {
	using concerned = entity_types_passing<concerned_with>;

	e.constrained_dispatch<concerned>([this](const auto& typed_handle) {
		specific_infer_cache_for(typed_handle);
	});
}

void organism_cache::reserve_caches_for_entities(const std::size_t) {

}
