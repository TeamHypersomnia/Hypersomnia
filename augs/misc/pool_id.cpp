#include <tuple>
#include "pool_id.h"
#include "augs/ensure.h"
#include <cstring>

namespace augs {
	std::ostream& operator<<(std::ostream& out, const augs::raw_pool_id &x) {
		out << "(" << x.pool.indirection_index << ";" << x.pool.version;
#if USE_NAMES_FOR_IDS
		out << ";" << x.get_debug_name();
#endif
		out << ")";
		return out;
	}

	raw_pool_id::raw_pool_id() {
		pool.version = 0;
		pool.indirection_index = -1;
#if USE_NAMES_FOR_IDS
		set_debug_name("unset");
#endif
	}

	void raw_pool_id::unset() {
		*this = raw_pool_id();
	}

	void raw_pool_id::set_debug_name(std::string s) {
#if USE_NAMES_FOR_IDS
		debug_name.assign(s.begin(), s.end());
#endif
	}

	std::string raw_pool_id::get_debug_name() const {
#if USE_NAMES_FOR_IDS
		return std::string(debug_name.begin(), debug_name.end());
#else
		return std::string();
#endif
	}

	bool raw_pool_id::operator==(const raw_pool_id& b) const {
		return std::make_tuple(pool.version, pool.indirection_index) == std::make_tuple(b.pool.version, b.pool.indirection_index);
	}

	bool raw_pool_id::operator!=(const raw_pool_id& b) const {
		return !operator==(b);
	}

	bool raw_pool_id::operator<(const raw_pool_id& b) const {
		return std::make_tuple(pool.version, pool.indirection_index) < std::make_tuple(b.pool.version, b.pool.indirection_index);
	}
}
