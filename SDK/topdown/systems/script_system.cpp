#include "script_system.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"

#include <lua/lua.hpp>
#include <luabind/luabind.hpp>

#include "../components/transform_component.h"
#include "../components/render_component.h"

script_system::script_system() : lua_state(luaL_newstate()) {
	luabind::open(lua_state);
	luabind::module(lua_state)[
		luabind::class_<world>("world")
			.def(luabind::constructor<>())
			.def("create_entity", &world::create_entity)
			.def("delete_entity", &world::delete_entity),

			luabind::class_<augmentations::vec2<>>("vec2")
			.def(luabind::constructor<float, float>()),

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