#pragma once
#define USE_NAMES_FOR_IDS 0

#if USE_NAMES_FOR_IDS
#include "augs/misc/constant_size_vector.h"
#endif

#include <string>
namespace augs {
	class raw_pool_id {
	public:
#if USE_NAMES_FOR_IDS
		constant_size_vector<char, 40> debug_name;
#endif
		int version = -1;
		int indirection_index = -1;

		raw_pool_id();

		void unset();

		void set_debug_name(std::string s);

		std::string get_debug_name() const;

		bool operator==(const raw_pool_id& b) const;
		bool operator!=(const raw_pool_id& b) const;
		bool operator<(const raw_pool_id& b) const;

		friend std::ostream& operator<<(std::ostream& out, const raw_pool_id &x);

		template <class Archive>
		void serialize(Archive& ar) {
#if USE_NAMES_FOR_IDS
			ar(CEREAL_NVP(debug_name));
#endif
			ar(CEREAL_NVP(version), CEREAL_NVP(indirection_index));
		}
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

		template<class B>
		bool operator<(const B& b) const {
			return indirection_index < b.indirection_index;
		}
	};

	template<class T>
	class pool_id : public raw_pool_id {
	public:
		typedef T element_type;

		operator unversioned_id<T>() const {
			unversioned_id<T> un;
			un.indirection_index = indirection_index;
			return un;
		}

		using raw_pool_id::raw_pool_id;
		using raw_pool_id::operator==;
		using raw_pool_id::operator!=;
		using raw_pool_id::operator<;
	};

	template<class T>
	struct make_pool_id { typedef pool_id<T> type; };
}