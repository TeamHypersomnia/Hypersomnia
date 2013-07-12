#pragma once
#include <unordered_map>

namespace augmentations {
	namespace util {
		template <class container>
		class type_mapper {
			container raw;

		public:
			template <typename type_in>
			void add(const type_in& in = type_in()) {
				raw.insert(typeid(type_in).hash_code, in);
			}

			template <typename type_in>
			void erase() {
				raw.erase(typeid(type_in).hash_code);
			}

			template <typename type_in>
			type_in& get() {
				return *static_cast<type_in*>(raw.at(typeid(type_in).hash_code));
			}
			
			//template <typename type_in>
			//type_in& get(size_t hash) {
			//	return raw[hash];
			//}
			
			container& mapper() {
				return raw;
			}

			const container& get_mapper() const {
				return raw;
			}
		};
	}
}