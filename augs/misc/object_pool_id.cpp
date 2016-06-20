#include "object_pool_id.h"
#include <tuple>

namespace augs {
	object_pool_id::object_pool_id() {
		set_debug_name("unset");
	}
	void object_pool_id::unset() {
		*this = object_pool_id();
	}

	void object_pool_id::set_debug_name(std::string s) {
#ifdef USE_NAMES_FOR_IDS
		ensure(s.size() < sizeof(debug_name) / sizeof(char));
		strcpy(debug_name, s.c_str());
#endif
	}

	std::string object_pool_id::get_debug_name() const {
#ifdef USE_NAMES_FOR_IDS
		return debug_name;
#else
		ensure(0);
#endif
	}

	bool object_pool_id::operator==(const object_pool_id& b) const {
		return std::make_tuple(version, indirection_index) == std::make_tuple(b.version, b.indirection_index);
	}

	bool object_pool_id::operator!=(const object_pool_id& b) const {
		return !operator==(b);
	}

	bool object_pool_id::operator<(const object_pool_id& b) const {
		return std::make_tuple(version, indirection_index) < std::make_tuple(b.version, b.indirection_index);
	}
}