#include "storage_for_components_and_aggregates.h"
#include "game/all_component_includes.h"

namespace augs {

	template<class...components>
	size_t storage_for_components_and_aggregates<components...>::aggregates_count() const {
		return pool_for_aggregates.size();
	}

	template<class...components>
	void storage_for_components_and_aggregates<components...>::reserve_storage_for_aggregates(size_t n) {
		pool_for_aggregates.initialize(n);

		auto r = [this, n](auto elem) {
			std::get<object_pool<decltype(elem)>>(pools_for_components).initialize(n);
		};

		for_each_type<components...>(r);
	}

	template<class...components>
	typename storage_for_components_and_aggregates<components...>::aggregate_id 
		storage_for_components_and_aggregates<components...>::clone_aggregate(typename storage_for_components_and_aggregates<components...>::aggregate_id aggregate) {
		
		auto new_aggregate = pool_for_aggregates.allocate();

		auto& from = *aggregate;
		auto& to = *new_aggregate;

		for_each_type<components...>([this, &from, &to](auto c) {
			auto* maybe_component = from.find<decltype(c)>();

			if (maybe_component) {
				ensure(to.find<decltype(c)>() == nullptr);

				writable_id<decltype(c)>(to) = allocate_component<decltype(c)>(*maybe_component);
			}
		});

		return new_aggregate;
	}

	template<class...components>
	void storage_for_components_and_aggregates<components...>::free_aggregate(aggregate_id aggregate) {
		pool_for_aggregates.free(aggregate);
	}

	template<class...components>
	const configurable_components<components...>& storage_for_components_and_aggregates<components...>::component_aggregate::get_definition() const {
		configurable_components<components...> result;

		for_each_type<components...>([this, &result](auto elem) {
			const auto const* p = this;

			if (p->find<decltype(elem)>() != nullptr) {
				result.add(p->get<decltype(elem)>());
			}
		});

		return result;
	}

}

#include "components_instantiation.h"