#pragma once
#include <unordered_set>
#include <limits>

namespace augs {
	namespace misc {
		template <typename T>
		class id_generator {
			T counter = T(0);

			std::unordered_set<T> registered_ids;
		public:
			bool can_be_generated() {
				return !(registered_ids.size() == std::numeric_limits<T>::max());
			}

			T generate_id() {
				while (exists(counter)) 
					++counter;

				registered_ids.insert(counter);
				return counter;
			}
			
			bool release_id(T id) {
				return registered_ids.erase(counter) > 0u;
			}

			bool exists(T id) {
				return registered_ids.find(counter) != registered_ids.end();
			}

			static luabind::scope bind(const char* class_name) {
				return
					luabind::class_<id_generator<T>>(class_name)
					.def(luabind::constructor<>())
					.def("can_be_generated", &can_be_generated)
					.def("generate_id", &generate_id)
					.def("release_id", &release_id)
					.def("exists", &exists);
			}
		};
	}
}