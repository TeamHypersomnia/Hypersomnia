#pragma once
#include <sstream>
#include "augs/templates/hash_templates.h"

namespace augs {
	template <class size_type, class... keys>
	struct unversioned_id {
		size_type indirection_index = static_cast<size_type>(-1);

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

	template <class size_type, class... keys>
	struct pool_undo_free_input;

	template <class size_type, class... keys>
	struct pooled_object_id {
		using undo_free_type = pool_undo_free_input<size_type, keys...>;

		// GEN INTROSPECTOR struct augs::pooled_object_id class size_type class... keys
		size_type version = 0;
		size_type indirection_index = static_cast<size_type>(-1);
		// END GEN INTROSPECTOR

		friend std::ostream& operator<<(std::ostream& out, const pooled_object_id x) {
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

		auto get_cache_index() const {
			return indirection_index;
		}

		operator unversioned_id<size_type>() const {
			unversioned_id<size_type> un;
			un.indirection_index = indirection_index;
			return un;
		}
	};
}

namespace std {
	template <class S, class... K>
	struct hash<augs::pooled_object_id<S, K...>> {
		std::size_t operator()(const augs::pooled_object_id<S, K...> k) const {
			return augs::simple_two_hash(k.indirection_index, k.version);
		}
	};

	template <class S, class... K>
	struct hash<augs::unversioned_id<S, K...>> {
		std::size_t operator()(const augs::unversioned_id<S, K...> k) const {
			return std::hash<int>()(k.indirection_index);
		}
	};
}