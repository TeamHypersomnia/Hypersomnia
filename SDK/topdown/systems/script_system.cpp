#include "script_system.h"

#include "texture_baker/texture_baker.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"

#include <lua/lua.hpp>
#include <luabind/luabind.hpp>

#include "../components/transform_component.h"
#include "../components/render_component.h"

#include "../game/sprite_helper.h"

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

script_system::script_system() : lua_state(luaL_newstate()) {
	luabind::open(lua_state);
	luabind::module(lua_state)[

		luabind::class_<vec2<>>("vec2")
			.def(luabind::constructor<float, float>())
			.def_readwrite("x", &vec2<>::x)
			.def_readwrite("y", &vec2<>::y),

		luabind::class_<texture_baker::atlas>("atlas")
			.def(luabind::constructor<>())
			.def("build", &texture_baker::atlas::default_build),

		luabind::class_<sprite_helper>("sprite")
			.def(luabind::constructor<std::wstring, texture_baker::atlas&>())
			.def_readwrite("size", &sprite_helper::size),

		luabind::class_<world>("world")
			.def(luabind::constructor<>())
			.def("create_entity", &world::create_entity)
			.def("delete_entity", &world::delete_entity),

		luabind::class_<components::render>("render")
			.def(luabind::constructor<unsigned, sprite_helper*, unsigned>()),

		luabind::class_<components::transform>("transform")
			.def(luabind::constructor<augmentations::vec2<>, float>()),

		luabind::class_<entity>("entity")
			.def("clear", &entity::clear)
			.def("add", &entity::add<components::render>)
			.def("get_render", &entity::get<components::render>)
			.def("find_render", &entity::find<components::render>)
			.def("add", &entity::add<components::transform>)
			.def("get_transform", &entity::get<components::transform>)
			.def("find_transform", &entity::find<components::transform>)
	];
}

script_system::~script_system() {
	lua_close(lua_state);
}

void script_system::process_entities(world& owner) {


}