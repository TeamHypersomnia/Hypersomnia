#pragma once
#include <cstddef>

namespace augs {
	namespace detail {
		template <class Archive, class Serialized>
		void read_raw_bytes(Archive& ar, Serialized* const location, const std::size_t object_count);

		template <class Archive, class Serialized>
		void write_raw_bytes(Archive& ar, const Serialized* const location, const std::size_t object_count);
	}

	template <class Archive, class Serialized>
	void read_bytes(Archive&, Serialized&);

	template <class Archive, class Serialized>
	void write_bytes(Archive&, const Serialized&);
}