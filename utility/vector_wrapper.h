#pragma once
#include <vector>
#include "ptr_wrapper.h"

namespace augmentations {
	namespace util {
		/* vector wrapper that is used to faciliate binding to lua */
		template<class value>
		struct vector_wrapper {
			std::vector<value> raw;
			
			void add(value v) {
				raw.push_back(v);
			}

			void push_back(const value& v) {
				raw.push_back(v);
			}

			ptr_wrapper<value> data() {
				return raw.data();
			}

			static luabind::scope bind(const char* name) {
				return luabind::class_<vector_wrapper<value>>("float_vector")
					.def(luabind::constructor<>())
					.def("add", &add)
					.def("push_back", &push_back)
					.def("data", &data);
			}
		};
	}
}