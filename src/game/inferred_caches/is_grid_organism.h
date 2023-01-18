#pragma once

template <class E>
bool is_grid_organism(const E& typed_handle) {
	if constexpr(E::template has<invariants::movement_path>()) {
		const auto& movement_path = typed_handle.template get<invariants::movement_path>();
		const auto& origin = typed_handle.template get<components::movement_path>().origin;
		return typed_handle.get_cosmos()[origin].alive() && movement_path.organism_wandering.is_enabled;
	}

	return false;
}

