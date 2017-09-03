#pragma once
#include <experimental\filesystem>

namespace augs {
	using path_type = std::experimental::filesystem::path;
}

namespace std {
	template <>
	struct hash<augs::path_type> {
		size_t operator()(const augs::path_type& k) const {
			return std::experimental::filesystem::hash_value(k);
		}
	};
}