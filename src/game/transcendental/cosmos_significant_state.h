#pragma once
#include "game/transcendental/cosmos_metadata.h"

using cosmos_base = put_all_components_into_t<augs::operations_on_all_components_mixin, cosmos>;

class cosmos_significant_state {
	// GEN INTROSPECTOR class cosmos_significant_state
	friend class cosmos;
	friend class cosmic_delta;
	friend struct augs::introspection_access;

	typename cosmos_base::aggregate_pool_type pool_for_aggregates;
	typename cosmos_base::dynamic_component_pools_type pools_for_components;
public:
	cosmos_metadata meta;
	// END GEN INTROSPECTOR

	void clear();

	std::size_t get_first_mismatch_pos(const cosmos_significant_state&) const;

	bool operator==(const cosmos_significant_state&) const;
	bool operator!=(const cosmos_significant_state&) const;
};