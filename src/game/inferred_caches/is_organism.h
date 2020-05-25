#pragma once

template <class E>
bool is_organism(const E& typed_handle) {
	if constexpr(E::template has<invariants::movement_path>()) {
		const auto& movement_path = typed_handle.template get<invariants::movement_path>();
		return movement_path.organism_wandering.is_enabled;
	}

	return false;
}

