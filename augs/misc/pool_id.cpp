#include <tuple>
#include "pool_id.h"
#include "augs/ensure.h"
#include <cstring>
#include <sstream>

namespace augs {
	std::ostream& operator<<(std::ostream& out, const augs::raw_pool_id &x) {
		out << "(" << x.pool.indirection_index << ";" << x.pool.version;
		out << ")";
		return out;
	}

	raw_pool_id::raw_pool_id() {
		pool.version = 0;
		pool.indirection_index = -1;
	}

	void raw_pool_id::unset() {
		*this = raw_pool_id();
	}

	bool raw_pool_id::operator==(const raw_pool_id& b) const {
		return std::make_tuple(pool.version, pool.indirection_index) == std::make_tuple(b.pool.version, b.pool.indirection_index);
	}

	bool raw_pool_id::operator!=(const raw_pool_id& b) const {
		return !operator==(b);
	}
}
