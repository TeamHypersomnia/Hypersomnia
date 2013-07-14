#pragma once
#include <typeinfo>
#include <vector>

namespace augmentations {
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


		struct type_pack;
		template<typename...>
		struct templated_list {
			static type_pack get();
		};
		
		template<typename t, typename... rest> 
		extern void get_types(templated_list<t, rest...>, std::vector<base_type>& v);

		template<typename t, typename... rest>
		void get_types(templated_list<t, rest...>, std::vector<base_type>& v) {
			base_type info; 
			info.set<t>();
			v.push_back(info);
			get_types(templated_list<rest...>(), v);
		}

		extern void get_types(templated_list<>, std::vector<base_type>&);

		template<class... args>
		struct templated_add { 
			template<class... types> void add(args...);
			virtual void add_n(const type_pack&, args...) = 0;
		};

		template<class... args>
		struct templated_remove  { 
			template<class... types> void remove(args...);
			virtual void remove_n(const type_pack&, args...) = 0;
		};

		/* faciliates adding/removing operations on raw type vector, derives from templated_add/templated_remove */
		struct type_pack : public templated_add<>, public templated_remove<> {
			std::vector<base_type> raw_types;
		
			void add_n(const type_pack&) override;
			void remove_n(const type_pack&) override;
		};


		template<class... args>
		template<class... types> 
		void templated_add<args...>::add(args... arguments) { 
			type_pack v;
			get_types(templated_list<types...>(), v.raw_types); 
			add_n(v, arguments...); 
		}

		template<class... args>
		template<class... types> 
		void templated_remove<args...>::remove(args... arguments) { 
			type_pack v;
			get_types(templated_list<types...>(), v.raw_types); 
			remove_n(v, arguments...); 
		}

		template<typename... args>
		type_pack templated_list<args...>::get() {
			type_pack ret;
			get_types(templated_list<args...>(), ret.raw_types);
			return ret;
		}
	}
}
