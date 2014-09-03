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

			int size() {
				return raw.size();
			}

			value at(int i) {
				return raw.at(i);
			}

			static luabind::scope bind(const char* name) {
				return luabind::class_<vector_wrapper<value>>(name)
					.def(luabind::constructor<>())
					.def("add", &add)
					.def("push_back", &push_back)
					.def("size", &size)
					.def("at", &at)
					.def("data", &data);
			}

			static luabind::scope bind_string(const char* name) {
				return luabind::class_<std::basic_string<value>>(name)
					.def(luabind::constructor<>())
					.def("add", (&std::basic_string<value>::push_back))
					.def("size", (size_t(__thiscall std::basic_string<value>::*) ())(&std::basic_string<value>::size))
					.def("at", (value&(__thiscall std::basic_string<value>::*) (size_t))(&std::basic_string<value>::at))
					.def("clear", &std::basic_string<value>::clear)
					.def("data", (value* (__thiscall std::basic_string<value>::*) ()) (&std::basic_string<value>::data));
			}

			static luabind::scope bind_vector(const char* name) {
				return luabind::class_<std::vector<value>>(name)
					.def(luabind::constructor<>())
					.def("add", (void(__thiscall std::vector<value>::*) (const value&))(&std::vector<value>::push_back))
					.def("size", (size_t(__thiscall std::vector<value>::*) ())(&std::vector<value>::size))
					.def("at", (value&(__thiscall std::vector<value>::*) (size_t))(&std::vector<value>::at))
					.def("clear", &std::vector<value>::clear)
					.def("data", (value* (__thiscall std::vector<value>::*) ()) (&std::vector<value>::data));
			}
		};

		extern vector_wrapper<wchar_t> towchar_vec(const std::wstring& s);
	}
}