#pragma once
namespace resources {}
namespace components {}
namespace messages {}

namespace augs {
	namespace graphics {}
	namespace misc {}
}

namespace helpers {}

using namespace resources;
using namespace components;
using namespace augs;
using namespace misc;

using namespace graphics;
using namespace helpers;
using namespace messages;

namespace luabind
{
	template <>
	struct default_converter<std::wstring>
		: native_converter_base<std::wstring>
	{
		static int compute_score(lua_State * s, int index) {
			return lua_type(s, index) == LUA_TSTRING ? 0 : -1;
		}

		std::wstring to_cpp_deferred(lua_State * s, int index) {
			std::string utf8str(lua_tostring(s, index));
			return std::wstring(utf8str.begin(), utf8str.end());
		}

		void to_lua_deferred(lua_State * s, const std::wstring & wstr) {
			std::string str(wstr.begin(), wstr.end());
			lua_pushstring(s, str.c_str());
		}
	};
	/**/

	template <>
	struct default_converter<const std::wstring>
		: default_converter<std::wstring>
	{};

	template <>
	struct default_converter<const std::wstring&>
		: default_converter<std::wstring>
	{};
}

template<class T>
static luabind::scope bind_vector_wrapper(const char* name) {
	typedef vector_wrapper<T> V;
	return luabind::class_<vector_wrapper<T>>(name)
		.def(luabind::constructor<>())
		.def("add", &V::add)
		.def("push_back", &V::push_back)
		.def("size", &V::size)
		.def("at", &V::at)
		.def("data", &V::data);
}

template<class T>
static luabind::scope bind_vector_wrapper_as_string(const char* name) {
	return luabind::class_<std::basic_string<T>>(name)
		.def(luabind::constructor<>())
		.def("add", (&std::basic_string<T>::push_back))
		.def("size", (size_t(__thiscall std::basic_string<T>::*) ())(&std::basic_string<T>::size))
		.def("at", (T&(__thiscall std::basic_string<T>::*) (size_t))(&std::basic_string<T>::at))
		.def("clear", &std::basic_string<T>::clear)
		.def("data", (T* (__thiscall std::basic_string<T>::*) ()) (&std::basic_string<T>::data));
}

template<class T>
static luabind::scope bind_stdvector(const char* name) {
	return luabind::class_<std::vector<T>>(name)
		.def(luabind::constructor<>())
		.def("add", (void(__thiscall std::vector<T>::*) (const T&))(&std::vector<T>::push_back))
		.def("size", (size_t(__thiscall std::vector<T>::*) ())(&std::vector<T>::size))
		.def("at", (T&(__thiscall std::vector<T>::*) (size_t))(&std::vector<T>::at))
		.def("clear", &std::vector<T>::clear)
		.def("reserve", &std::vector<T>::reserve)
		.def("data", (T* (__thiscall std::vector<T>::*) ()) (&std::vector<T>::data));
}


#include "misc/map_wrapper.h"

template<class key, class value>
static luabind::scope bind_map_wrapper(const char* name) {
	typedef augs::map_wrapper<key, value> V;

	return
		luabind::class_<V::find_result>((std::string(name) + std::string("find_result")).c_str())
		.property("value", &V::find_result::get_value)
		.def_readwrite("found", &V::find_result::found),

		luabind::class_<V>(name)
		.def(luabind::constructor<>())
		.def("add", &V::add)
		.def("insert", &V::insert)
		.def("at", &V::at)
		.def("get", &V::get)
		.def("find", &V::find)
		.def("remove", &V::remove)
		.def("size", &V::size);
}

