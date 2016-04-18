#pragma once
#include "stdafx.h"
#include "bindings/bindings.h"

#include "misc/vector_wrapper.h"

#include "bind_game_and_augs.h"
#include "augs/script.h"
#include "components/physics_component.h"

#include "entity_system/entity_id.h"
#include "augs/lua_state_wrapper.h"

int bitflag(lua_State* L) {
	int result = 1 << luabind::object_cast<int>(luabind::object(luabind::from_stack(L, 1)));
	lua_pushinteger(L, result);
	return 1;
}

namespace bindings {
	extern luabind::scope
		_audio(),

		_id_generator(),
		_minmax(),
		_vec2(),
		_value_animator(),
		_b2Filter(),
		_rgba(),
		_rect_ltrb(),
		_rect_xywh(),
		_glwindow(),
		_script(),
		_texture(),
		_animation(),
		_world(),
		_sprite(),
		_polygon(),

		_timer(),

		_network_binding(),

		_particle(),
		_emission(),
		_particle_effect(),

		_all_messages(),
		_all_components(),

		_entity(),
		_body_helper(),

		_opengl_binding(),
		_random_binding(),
		_all_systems(),

		_text(),

		_file_watcher(),

		_utilities()
		;
}

double get_meters_to_pixels() {
	return METERS_TO_PIXELS;
}

void set_meters_to_pixels(double val) {
	METERS_TO_PIXELS = val;
	PIXELS_TO_METERS = 1.0 / METERS_TO_PIXELS;
	METERS_TO_PIXELSf = float(val);
	PIXELS_TO_METERSf = 1.0f / METERS_TO_PIXELSf;
}

#include "game_world.h"

void game_world::bind_this_to_lua_global(lua_state_wrapper& lua, std::string global) {
	lua.global_ptr(global, this);
}

void bind_game_and_augs(augs::lua_state_wrapper& wrapper) {
	using namespace resources;
	using namespace helpers;

	auto& raw = wrapper.raw;
	luabind::open(raw);

	lua_register(raw, "bitflag", bitflag);
	luabind::module(raw)[
		luabind::class_<ptr_wrapper<float>>("float_ptr"),

			bind_stdvector<std::string>("string_vector"),
			bind_stdvector<std::wstring>("wstring_vector"),
			bind_stdvector<vec2>("vec2_vector"),
			bind_stdvector<entity_id>("entity_ptr_vector"),
			
			bind_vector_wrapper<int>("int_vector"),
			bind_vector_wrapper<float>("float_vector"),
			
			bindings::_id_generator(),
			bindings::_minmax(),
			bindings::_value_animator(),
			//bindings::_b2Filter(),
			bindings::_rgba(),
			bindings::_rect_ltrb(),
			bindings::_rect_xywh(),
			bindings::_glwindow(),
			bindings::_script(),
			//bindings::_texture(),
			//bindings::_animation(),
			bindings::_world(),

			luabind::class_<game_world, augs::world>("game_world")
			.def(luabind::constructor<overworld&>()),

			//bindings::_sprite(),
			//bindings::_polygon(),
			//
			//bindings::_network_binding(),
			//
			//bindings::_particle(),
			//bindings::_emission(),
			//bindings::_particle_effect(),

			//bindings::_all_messages(),
			//bindings::_all_components(),
			//bindings::_all_systems(),

			//bindings::_entity(),
			//bindings::_body_helper(),

			bindings::_opengl_binding(),

			luabind::def("clamp", &augs::get_clamp<float>),
			bindings::_random_binding(),

			luabind::def("open_editor", lua_state_wrapper::open_editor),
			//luabind::def("get_meters_to_pixels", get_meters_to_pixels),
			//luabind::def("set_meters_to_pixels", set_meters_to_pixels),
			luabind::def("get_executable_path", window::get_executable_path),
			luabind::def("remove_filename_from_path", window::remove_filename_from_path),

			luabind::class_<std::string>("std_string")
			.def("c_str", &std::string::c_str),

			bindings::_file_watcher(),

			bindings::_text(),

			bindings::_timer(),
			bindings::_utilities(),
			bindings::_vec2()
	];

	wrapper.global("THIS_LUA_STATE", wrapper);

	//luabind::bind_class_info(raw);
}