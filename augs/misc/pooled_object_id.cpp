#include <tuple>
#include "pooled_object_id.h"
#include "augs/ensure.h"
#include <cstring>
#include <sstream>

namespace augs {
	std::ostream& operator<<(std::ostream& out, const augs::pooled_object_raw_id &x) {
		out << "(" << x.indirection_index << ";" << x.version;
		out << ")";
		return out;
	}

	void pooled_object_raw_id::unset() {
		*this = pooled_object_raw_id();
	}

	bool pooled_object_raw_id::operator==(const pooled_object_raw_id& b) const {
		return std::make_tuple(version, indirection_index) == std::make_tuple(b.version, b.indirection_index);
	}

	bool pooled_object_raw_id::operator!=(const pooled_object_raw_id& b) const {
		return !operator==(b);
	}
}
