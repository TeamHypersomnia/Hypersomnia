#pragma once
#include <iosfwd>
#include "augs/templates/hash_templates.h"

namespace augs {
	class pooled_object_raw_id {
	public:
		// GEN INTROSPECTOR class augs::pooled_object_raw_id
		unsigned version = 0;
		int indirection_index = -1;
		// END GEN INTROSPECTOR

		void unset();
		bool is_set() const;

		bool operator==(const pooled_object_raw_id& b) const;
		bool operator!=(const pooled_object_raw_id& b) const;

		friend std::ostream& operator<<(std::ostream& out, const pooled_object_raw_id &x);
	};

	template<class T>
	struct unversioned_id {
		int indirection_index = -1;

		template<class B>
		bool operator==(const B& b) const {
			return indirection_index == b.indirection_index;
		}

		template<class B>
		bool operator!=(const B& b) const {
			return indirection_index != b.indirection_index;
		}
	};

	template <class T>
	class pooled_object_id : public pooled_object_raw_id {
	public:
		// GEN INTROSPECTOR class augs::pooled_object_id class T
		// INTROSPECT BASE augs::pooled_object_raw_id
		// END GEN INTROSPECTOR

		typedef T element_type;

		operator unversioned_id<T>() const {
			unversioned_id<T> un;
			un.indirection_index = indirection_index;
			return un;
		}

		using pooled_object_raw_id::pooled_object_raw_id;
		using pooled_object_raw_id::operator==;
		using pooled_object_raw_id::operator!=;
	};

	template<class T>
	struct make_pooled_object_id { 
		typedef pooled_object_id<T> type; 
	};
}

namespace std {
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