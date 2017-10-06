#include <cstring>
#include <sstream>

#include "augs/ensure.h"
#include "augs/misc/pooled_object_id.h"

namespace augs {
	bool unversioned_id_base::operator==(const unversioned_id_base& b) const {
		return indirection_index == b.indirection_index;
	}

	bool unversioned_id_base::operator!=(const unversioned_id_base& b) const {
		return !operator==(b);
	}

	bool unversioned_id_base::is_set() const {
		return *this != unversioned_id_base();
	}

	std::ostream& operator<<(std::ostream& out, const augs::pooled_object_id_base &x) {
		out << "(" << x.indirection_index << ";" << x.version;
		out << ")";
		return out;
	}

	void pooled_object_id_base::unset() {
		*this = pooled_object_id_base();
	}

	bool pooled_object_id_base::is_set() const {
		return *this != pooled_object_id_base();
	}

	bool pooled_object_id_base::operator==(const pooled_object_id_base& b) const {
		return version == b.version && indirection_index == b.indirection_index;
	}

	bool pooled_object_id_base::operator!=(const pooled_object_id_base& b) const {
		return !operator==(b);
	}
}
