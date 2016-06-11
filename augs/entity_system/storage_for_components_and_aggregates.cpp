#include "storage_for_components_and_aggregates.h"

namespace augs {
	template<class...components>
	storage_for_components_and_aggregates<components...>::aggregate_id 
		storage_for_components_and_aggregates<components...>::clone_aggregate(aggregate_id aggregate) {
		auto new_aggregate = pool_for_aggregates.allocate(allocator.to);

		component_cloner allocator{ *this };
		allocator.from = *aggregate;
		allocator.to = *new_aggregate;

		return new_aggregate;
	}

	template<class...components>
	void storage_for_components_and_aggregates<components...>::free_aggregate(aggregate_id aggregate) {
		pool_for_aggregates.free(aggregate);
	}

}

#include "components_instantiation.h"