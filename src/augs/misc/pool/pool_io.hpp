#pragma once
#include "augs/misc/pool/pool.h"

#include "augs/readwrite/byte_readwrite_declaration.h"

namespace augs {
	template <class A, template <class> class B, class C, class D, class... E>
	template <class Archive>
	void pool<A, B, C, D, E...>::write_object_bytes(Archive& ar) const {
		auto w = [&ar](const auto& object) {
			augs::write_capacity_bytes(ar, object);
			augs::write_bytes(ar, object);
		};

		w(objects);
		w(slots);
		w(indirectors);
		w(free_indirectors);
	}

	template <class A, template <class> class B, class C, class D, class... E>
	template <class Archive>
	void pool<A, B, C, D, E...>::read_object_bytes(Archive& ar) {
		auto r = [&ar](auto& object) {
			augs::read_capacity_bytes(ar, object);
			augs::read_bytes(ar, object);
		};

		r(objects);
		r(slots);
		r(indirectors);
		r(free_indirectors);

		if constexpr(has_synchronized_arrays) {
			synchronized_arrays.for_each_container(
				[&](auto& container) {
					container.resize(objects.size());
				}
			);
		}
	}
}
