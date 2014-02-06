#pragma once
namespace resources {}
namespace components {}
namespace messages {}

namespace augs {
	namespace entity_system {}
	namespace graphics {}
	namespace misc {}
}

namespace helpers {}

using namespace resources;
using namespace components;
using namespace augs;
using namespace misc;
using namespace entity_system;
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

		std::wstring from(lua_State * s, int index) {
			std::string utf8str(lua_tostring(s, index));
			return std::wstring(utf8str.begin(), utf8str.end());
		}

		void to(lua_State * s, const std::wstring & str) {
			lua_pushstring(s, std::string(str.begin(), str.end()).c_str());
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