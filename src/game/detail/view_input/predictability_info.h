#pragma once
#include "game/cosmos/entity_id.h"

enum class predictability_type {
	ALWAYS,
	NEVER,
	ONLY_BY
};

struct predictability_info {
	predictability_type type = predictability_type::ALWAYS;
	entity_id only_by;

	void set_only_by(const entity_id& b) {
		only_by = b;
		type = predictability_type::ONLY_BY;
	}
};
