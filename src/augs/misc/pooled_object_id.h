#pragma once
#include <iosfwd>
#include "augs/templates/hash_templates.h"

namespace augs {
	template <class T, class size_type>
	struct unversioned_id {
		using mapped_type = T;

		size_type indirection_index = -1;

		bool operator==(const unversioned_id& b) const {
			return indirection_index == b.indirection_index;
		}

		bool operator!=(const unversioned_id& b) const {
			return !operator==(b);
		}

		bool is_set() const {
			return *this != unversioned_id();
		}
	};

	template <class T, class size_type>
	struct pooled_object_id {
		// GEN INTROSPECTOR struct augs::pooled_object_id class T class size_type
		size_type version = 0;
		size_type indirection_index = -1;
		// END GEN INTROSPECTOR

		using mapped_type = T;

		friend std::ostream& operator<<(std::ostream& out, const pooled_object_id& x) {
			return out << "(" << x.indirection_index << ";" << x.version << ")";
		}

		void unset() {
			*this = pooled_object_id();
		}

		bool is_set() const {
			return *this != pooled_object_id();
		}

		bool operator==(const pooled_object_id& b) const {
			return version == b.version && indirection_index == b.indirection_index;
		}

		bool operator!=(const pooled_object_id& b) const {
			return !operator==(b);
		}

		operator unversioned_id<T, size_type>() const {
			unversioned_id<T, size_type> un;
			un.indirection_index = indirection_index;
			return un;
		}
	};
}

namespace std {
	template <class T>
	struct hash;

	template <class T, class S>
	struct hash<augs::pooled_object_id<T, S>> {
		std::size_t operator()(const augs::pooled_object_id<T, S>& k) const {
			return augs::simple_two_hash(k.indirection_index, k.version);
		}
	};

	template <class T, class S>
	struct hash<augs::unversioned_id<T, S>> {
		std::size_t operator()(const augs::unversioned_id<T, S> k) const {
			return std::hash<int>()(k.indirection_index);
		}
	};
}