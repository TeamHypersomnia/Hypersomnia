#include <cstring>
#include <sstream>

#include "augs/ensure.h"
#include "augs/misc/pooled_object_id.h"

namespace augs {
	std::ostream& operator<<(std::ostream& out, const augs::pooled_object_raw_id &x) {
		out << "(" << x.indirection_index << ";" << x.version;
		out << ")";
		return out;
	}

	void pooled_object_raw_id::unset() {
		*this = pooled_object_raw_id();
	}

	bool pooled_object_raw_id::is_set() const {
		return *this != pooled_object_raw_id();
	}

	bool pooled_object_raw_id::operator==(const pooled_object_raw_id& b) const {
		return version == b.version && indirection_index == b.indirection_index;
	}

	bool pooled_object_raw_id::operator!=(const pooled_object_raw_id& b) const {
		return !operator==(b);
	}
}
