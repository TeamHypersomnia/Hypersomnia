#pragma once
#include <cstddef>

namespace augs {
	template <class Archive, class Serialized>
	void read_bytes(Archive&, Serialized&);

	template <class Archive, class Serialized>
	void write_bytes(Archive&, const Serialized&);
}