#pragma once
#define USE_NAMES_FOR_IDS

#ifdef USE_NAMES_FOR_IDS
#include <string>
#include "ensure.h"
#endif

namespace augs {
	template<class owner_pool_type>
	class pool_id {
	public:
#ifdef USE_NAMES_FOR_IDS
		char debug_name[40];
#endif
		int version = 0xdeadbeef;
		int indirection_index = -1;

		pool_id() {
			set_debug_name("unset");
		}

		void unset() {
			*this = pool_id();
		}

		void set_debug_name(std::string s) {
#ifdef USE_NAMES_FOR_IDS
			ensure(s.size() < sizeof(debug_name) / sizeof(char));
			strcpy(debug_name, s.c_str());
#endif
		}

		std::string get_debug_name() const {
#ifdef USE_NAMES_FOR_IDS
			return debug_name;
#else
			ensure(0);
#endif
		}

		bool operator==(const pool_id& b) const {
			return std::make_tuple(version, indirection_index) == std::make_tuple(b.version, b.indirection_index);
		}

		bool operator!=(const pool_id& b) const {
			return !operator==(b);
		}

		bool operator<(const pool_id& b) const {
			return std::make_tuple(version, indirection_index) < std::make_tuple(b.version, b.indirection_index);
		}
	};

	template<class T>
	struct make_pool_id { typedef pool_id<T> type; };
}