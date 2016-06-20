#include "storage_for_components_and_aggregates.h"
#include "game/types_specification/all_component_includes.h"

namespace augs {
	template<class...components>
	size_t storage_for_components_and_aggregates<components...>::aggregates_count() const {
		return pool_for_aggregates.size();
	}

	template<class...components>
	void storage_for_components_and_aggregates<components...>::reserve_storage_for_aggregates(size_t n) {
		pool_for_aggregates.initialize_space(n);

		auto r = [this, n](auto elem) {
			std::get<object_pool<decltype(elem)>>(pools_for_components).initialize(n);
		};

		for_each_type<components...>(r);
	}

	template<class...components>
	typename storage_for_components_and_aggregates<components...>::aggregate_id 
		storage_for_components_and_aggregates<components...>::clone_aggregate(typename storage_for_components_and_aggregates<components...>::aggregate_id aggregate_id) {

		auto new_aggregate_id = pool_for_aggregates.allocate();
		auto& new_aggregate = pool_for_aggregates.get(new_aggregate_id);

		auto& from_handle = get_handle(aggregate_id);
		auto& to = new_aggregate;

		auto& aggregate = pool_for_aggregates.get(aggregate_id);
		to.removed_from_processing_subjects = aggregate.removed_from_processing_subjects;

		for_each_type<components...>([this, &from_handle, &to](auto c) {
			auto* maybe_component = from_handle.find<decltype(c)>();

			if (maybe_component) {
				ensure(to.find<decltype(c)>() == nullptr);

				to.get_id<decltype(c)>() = allocate_component<decltype(c)>(*maybe_component);
			}
		});

		new_aggregate_id.set_debug_name(aggregate_id.get_debug_name());

		return new_aggregate_id;
	}

	template<class...components>
	void storage_for_components_and_aggregates<components...>::free_aggregate(aggregate_id aggregate) {
		pool_for_aggregates.free(aggregate);
	}

	template<class...components>
	template<bool is_const>
	const configurable_components<components...>& storage_for_components_and_aggregates<components...>::basic_aggregate_handle<is_const>::get_definition() const {
		configurable_components<components...> result;

		for_each_type<components...>([this, &result](auto elem) {
			const auto const* p = this;

			if (p->find<decltype(elem)>() != nullptr) {
				result.set(p->get<decltype(elem)>());
			}
		});

		return result;
	}

	template<class...components>
	template<bool is_const>
	typename storage_for_components_and_aggregates<components...>::aggregate_id 
		storage_for_components_and_aggregates<components...>::basic_aggregate_handle<is_const>::get_id() const {
		return raw_id;
	}

}

#include "storage_instantiation.h"