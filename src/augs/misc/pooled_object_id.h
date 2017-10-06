#pragma once
#include <iosfwd>
#include "augs/templates/hash_templates.h"

namespace augs {
	class pooled_object_id_base {
	public:
		// GEN INTROSPECTOR class augs::pooled_object_id_base
		unsigned version = 0;
		unsigned indirection_index = -1;
		// END GEN INTROSPECTOR

		void unset();
		bool is_set() const;

		bool operator==(const pooled_object_id_base& b) const;
		bool operator!=(const pooled_object_id_base& b) const;

		friend std::ostream& operator<<(std::ostream& out, const pooled_object_id_base &x);
	};

	struct unversioned_id_base {
		unsigned indirection_index = -1;
		
		bool is_set() const;

		bool operator==(const unversioned_id_base& b) const;
		bool operator!=(const unversioned_id_base& b) const;
	};

	template <class T>
	struct unversioned_id : unversioned_id_base {
		using unversioned_id_base::unversioned_id_base;
		using unversioned_id_base::operator==;
		using unversioned_id_base::operator!=;
	};

	template <class T>
	class pooled_object_id : public pooled_object_id_base {
	public:
		// GEN INTROSPECTOR class augs::pooled_object_id class T
		// INTROSPECT BASE augs::pooled_object_id_base
		// END GEN INTROSPECTOR

		using mapped_type = T;

		operator unversioned_id<T>() const {
			unversioned_id<T> un;
			un.indirection_index = indirection_index;
			return un;
		}

		using pooled_object_id_base::pooled_object_id_base;
		using pooled_object_id_base::operator==;
		using pooled_object_id_base::operator!=;
	};
}

namespace std {
	template <class T>
	struct hash;

	template <class T>
	struct hash<augs::pooled_object_id<T>> {
		std::size_t operator()(const augs::pooled_object_id<T>& k) const {
			return augs::simple_two_hash(k.indirection_index, k.version);
		}
	};

	template <class T>
	struct hash<augs::unversioned_id<T>> {
		std::size_t operator()(const augs::unversioned_id<T> k) const {
			return std::hash<int>()(k.indirection_index);
		}
	};
}