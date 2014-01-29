#pragma once
#include <typeinfo>
#include <vector>

namespace augs {
	namespace entity_system {
		typedef size_t type_hash;

		struct base_type {
			size_t bytes;
			type_hash hash;

			template <class T>
			void set() {
				bytes = sizeof(T);
				hash = typeid(T).hash_code();
			}

			bool operator<(const base_type& b) const {
				return hash < b.hash;
			}

			bool operator==(const base_type& b) const {
				return hash == b.hash;
			}
		};

		typedef std::vector<base_type> type_pack;

		template<typename... args>
		struct templated_list {
			static type_pack get() {
				type_pack ret;
				get_types(templated_list<args...>(), ret);
				return ret;
			}
		};
		
		template<typename t, typename... rest> 
		extern void get_types(templated_list<t, rest...>, type_pack& v);

		template<typename t, typename... rest>
		void get_types(templated_list<t, rest...>, type_pack& v) {
			base_type info; 
			info.set<t>();
			v.push_back(info);
			get_types(templated_list<rest...>(), v);
		}

		extern void get_types(templated_list<>, type_pack&);
	}
}
