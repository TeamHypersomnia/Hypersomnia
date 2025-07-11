#pragma once
#include <cstdint>

template <class T, class P, class Cmp, class F, class S>
void in_order_of(T& container, P get_property, Cmp comparator, F order_callback, S& sorted_cache) {
	auto& sorted = sorted_cache;
	sorted.resize(container.size());

	{
		auto it = container.begin();

		for (uint32_t i = 0; i < container.size(); ++i) {
			sorted[i].criterion = get_property(*it);
			sorted[i].index = i;

			++it;
		}
	}

	std::sort(sorted.begin(), sorted.end(), [&](const auto& a, const auto& b) { return comparator(a.criterion, b.criterion); });

	for (const auto& entry : sorted) {
		order_callback(*(container.begin() + entry.index));
	}
}

template <class T, class P, class Cmp, class F>
void in_order_of(T&& container, P get_property, Cmp&& comparator, F&& order_callback) {
	using property_type = decltype(get_property(*container.begin()));

	struct sorted_entry {
		property_type criterion;
		uint32_t index = 0;
	};

	thread_local std::vector<sorted_entry> sorted;

	::in_order_of(
		std::forward<T>(container), 
		get_property, 
		std::forward<Cmp>(comparator), 
		std::forward<F>(order_callback), 
		sorted
	);
}
