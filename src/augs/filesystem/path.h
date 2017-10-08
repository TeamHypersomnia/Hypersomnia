#pragma once
#include <experimental\filesystem>
#include "augs/misc/templated_readwrite.h"

namespace augs {
	using path_type = std::experimental::filesystem::path;

	template <class Archive>
	void read_object(
		Archive& ar,
		path_type& storage
	) {
		std::string str;
		read(ar, str);
		storage = str;
	}

	template <class Archive>
	void write_object(
		Archive& ar,
		const path_type& storage
	) {
		write(ar, storage.string());
	}

	static_assert(has_readwrite_overloads_v<stream, path_type>);
}

namespace std {
	template <>
	struct hash<augs::path_type> {
		size_t operator()(const augs::path_type& k) const {
			return std::experimental::filesystem::hash_value(k);
		}
	};
}