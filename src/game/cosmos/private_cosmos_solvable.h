#pragma once
#include "game/cosmos/cosmos_solvable.h"
#include "game/cosmos/cosmos_solvable_access.h"
#include "game/cosmos/cosmos_solvable_inferred_access.h"

struct cosmic_functions_detail;

class private_cosmos_solvable {
	cosmos_solvable solvable;

public:
	private_cosmos_solvable() = default;
	explicit private_cosmos_solvable(const cosmic_pool_size_type reserved_entities) : solvable(reserved_entities) {};

	auto& get_solvable(cosmos_solvable_access) {
		return solvable;
	}

	const auto& get_solvable(cosmos_solvable_access) const {
		return solvable;
	}

	const auto& get_solvable() const {
		return solvable;
	}

	auto& get_solvable_inferred(cosmos_solvable_inferred_access) {
		return solvable.inferred;
	}

	const auto& get_solvable_inferred(cosmos_solvable_inferred_access) const {
		return solvable.inferred;
	}

	const auto& get_solvable_inferred() const {
		return solvable.inferred;
	}

	auto& get_global_solvable() {
		return solvable.significant.global;
	}

	const auto& get_global_solvable() const {
		return solvable.significant.global;
	}
};
