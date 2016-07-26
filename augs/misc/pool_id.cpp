#include <tuple>
#include "pool_id.h"
#include "augs/ensure.h"
#include <cstring>

namespace augs {
	std::ostream& operator<<(std::ostream& out, const augs::raw_pool_id &x) {
		out << "(" << x.indirection_index << ";" << x.version;
#ifdef USE_NAMES_FOR_IDS
		out << ";" << x.get_debug_name();
#endif
		out << ")";
		return out;
	}

	raw_pool_id::raw_pool_id() {
		set_debug_name("unset");
	}

	void raw_pool_id::unset() {
		*this = raw_pool_id();
	}

	void raw_pool_id::set_debug_name(std::string s) {
#ifdef USE_NAMES_FOR_IDS
		ensure(s.size() < sizeof(debug_name) / sizeof(char));
		strcpy(debug_name, s.c_str());
#endif
	}

	std::string raw_pool_id::get_debug_name() const {
#ifdef USE_NAMES_FOR_IDS
		return debug_name;
#else
		ensure(0);
#endif
	}

	bool raw_pool_id::operator==(const raw_pool_id& b) const {
		return std::make_tuple(version, indirection_index) == std::make_tuple(b.version, b.indirection_index);
	}

	bool raw_pool_id::operator!=(const raw_pool_id& b) const {
		return !operator==(b);
	}

	bool raw_pool_id::operator<(const raw_pool_id& b) const {
		return std::make_tuple(version, indirection_index) < std::make_tuple(b.version, b.indirection_index);
	}
}
