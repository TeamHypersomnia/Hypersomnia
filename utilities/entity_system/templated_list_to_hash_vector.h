#pragma once
#include <typeinfo>
#include <vector>

namespace augs {
	namespace entity_system {
		typedef std::vector<size_t> type_hash_vector;

		template<typename... args>
		struct templated_list_to_hash_vector {
			static type_hash_vector unpack() {
				type_hash_vector result;
				unpack_types(templated_list_to_hash_vector<args...>(), result);
				return result;
			}
		};
		
		template<typename t, typename... rest> 
		extern void unpack_types(templated_list_to_hash_vector<t, rest...>, type_hash_vector& v);

		template<typename t, typename... rest>
		void unpack_types(templated_list_to_hash_vector<t, rest...>, type_hash_vector& v) {
			v.push_back(typeid(t).hash_code());
			unpack_types(templated_list_to_hash_vector<rest...>(), v);
		}

		extern void unpack_types(templated_list_to_hash_vector<>, type_hash_vector&);
	}
}
