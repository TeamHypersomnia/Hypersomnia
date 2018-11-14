#pragma once
#include "view/audiovisual_state/systems/exploding_ring_system.h"

template <class C>
void exploding_ring_system::acquire_new_rings(const C& new_rings) {
	auto i = rings.size();
	rings.resize(rings.size() + new_rings.size());

	for (const auto& r : new_rings) {
		auto& n = rings[i++];
		n.in = r;
		n.time_of_occurence_seconds = global_time_seconds;
	}
}

template <class I>
void exploding_ring_system::acquire_new_ring(I&& in) {
	rings.emplace_back();
	
	auto& n = rings.back();
	n.in = std::forward<I>(in);
	n.time_of_occurence_seconds = global_time_seconds;
}