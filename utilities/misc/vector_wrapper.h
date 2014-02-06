#pragma once
#include <vector>
#include "ptr_wrapper.h"

namespace augs {
	namespace misc {
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
				return luabind::class_<vector_wrapper<value>>(name)
					.def(luabind::constructor<>())
					.def("add", &add)
					.def("push_back", &push_back)
					.def("data", &data);
			}

			static luabind::scope bind_vector(const char* name) {
				return luabind::class_<std::vector<value>>(name)
					.def(luabind::constructor<>())
					.def("add", (void(__thiscall std::vector<value>::*) (const value&))(&std::vector<value>::push_back))
					.def("size", (size_t(__thiscall std::vector<value>::*) ())(&std::vector<value>::size))
					.def("at", (value&(__thiscall std::vector<value>::*) (size_t))(&std::vector<value>::at))
					.def("data", (value* (__thiscall std::vector<value>::*) ()) (&std::vector<value>::data));
			}
		};
	}
}