#pragma once
#include "view/audiovisual_state/systems/exploding_ring_system.h"

template <class C>
void exploding_ring_system::acquire_new_rings(const C& new_rings) {
	for (const auto& r : new_rings) {
		if (container_full(rings)) {
			break;
		}

		auto& n = rings.emplace_back();
		n.in = r;
		n.time_of_occurence_seconds = global_time_seconds;
	}
}

template <class I>
void exploding_ring_system::acquire_new_ring(I&& in) {
	if (container_full(rings)) {
		return;
	}

	auto& n = rings.emplace_back();
	n.in = std::forward<I>(in);
	n.time_of_occurence_seconds = global_time_seconds;
}