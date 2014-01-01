#include "stdafx.h"
#include <luabind/class_info.hpp>

#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "script_system.h"

#include "../components/scriptable_component.h"
#include "../bindings/bindings.h"

#include "../messages/collision_message.h"
#include "../messages/damage_message.h"
#include "../messages/intent_message.h"


void set_world_reloading_script(resources::script* new_scr) {
	world_reloading_script = new_scr;
}

int bitor(lua_State* L) {
	int arg_count = lua_gettop(L);
	int result = 0;

	for (int i = 1; i <= arg_count; i++) {
		luabind::object obj(luabind::from_stack(L, i));

		if(luabind::type(obj) == LUA_TNUMBER) 
			result |= luabind::object_cast<int>(obj);
	}

	lua_pushinteger(L, result);
	return 1;
}

int bitflag(lua_State* L) {
	int result = 1 << luabind::object_cast<int>(luabind::object(luabind::from_stack(L, 1)));
	lua_pushinteger(L, result);
	return 1;
}


namespace bindings {
	extern luabind::scope
		_minmax(),
		_vec2(),
		_b2Filter(),
		_rgba(),
		_rect_ltrb(),
		_rect_xywh(),
		_glwindow(),
		_script(),
		_texture(),
		_animation(),
		_world(),
		_entity_ptr(),
		_sprite(),
		_polygon(),

		_particle(),
		_emission(),
		_particle_effect(),

		_intent_message(),
		_animate_message(),
		_particle_burst_message(),
		_collision_message(),
		_damage_message(),
		_destroy_message(),

		_render_component(),
		_transform_component(),
		_visibility_component(),
		_pathfinding_component(),
		_animate_component(),
		_camera_component(),
		_chase_component(),
		_children_component(),
		_crosshair_component(),
		_damage_component(),
		_gun_component(),
		_input_component(),
		_lookat_component(),
		_movement_component(),
		_particle_emitter_component(),
		_physics_component(),
		_steering_component(),
		_scriptable_component(),
		_behaviour_tree_component(),

		_entity(),
		_body_helper();
}

int the_callback(lua_State *L) {
	std::cout << lua_tostring(L, -1) << std::endl;
	return 0;
}

script_system::script_system() : lua_state(luaL_newstate()) {
	using namespace resources;
	using namespace topdown;

	luabind::open(lua_state);
	luabind::bind_class_info(lua_state);

	luaL_openlibs(lua_state);

	lua_register(lua_state, "bitor", bitor);
	lua_register(lua_state, "bitflag", bitflag);
	luabind::module(lua_state)[
			bindings::_minmax(),
			bindings::_vec2(),
			bindings::_b2Filter(),
			bindings::_rgba(),
			bindings::_rect_ltrb(),
			bindings::_rect_xywh(),
			bindings::_glwindow(),
			bindings::_script(),
			bindings::_texture(),
			bindings::_animation(),
			bindings::_world(),
			bindings::_entity_ptr(),
			bindings::_sprite(),
			bindings::_polygon(),

			bindings::_particle(),
			bindings::_emission(),
			bindings::_particle_effect(),
					  
			bindings::_intent_message(),
			bindings::_animate_message(),
			bindings::_particle_burst_message(),
			bindings::_collision_message(),
			bindings::_damage_message(),
			bindings::_destroy_message(),

			bindings::_render_component(),
			bindings::_transform_component(),
			bindings::_visibility_component(),
			bindings::_pathfinding_component(),
			bindings::_animate_component(),
			bindings::_camera_component(),
			bindings::_chase_component(),
			bindings::_children_component(),
			bindings::_crosshair_component(),
			bindings::_damage_component(),
			bindings::_gun_component(),
			bindings::_input_component(),
			bindings::_lookat_component(),
			bindings::_movement_component(),
			bindings::_particle_emitter_component(),
			bindings::_physics_component(),
			bindings::_steering_component(),
			bindings::_scriptable_component(),
			bindings::_behaviour_tree_component(), 

			bindings::_entity(),
			bindings::_body_helper(),

			luabind::def("set_world_reloading_script", &set_world_reloading_script)
	];

	luabind::set_pcall_callback(the_callback);
}

script_system::~script_system() {
	lua_close(lua_state);
}

void script_system::process_entities(world& owner) {
	auto target_copy = targets;
	for (auto it : target_copy) {
		auto& scriptable = it->get<components::scriptable>();
		if (!scriptable.available_scripts) continue;

		auto loop_event = scriptable.available_scripts->get_raw().find(components::scriptable::LOOP);
		
		if (loop_event != scriptable.available_scripts->get_raw().end()) {
			try {
				luabind::call_function<void>((*loop_event).second, it);
			}
			catch (std::exception compilation_error) {
				std::cout << compilation_error.what() << std::endl;
			}
		}
		//auto loop_event = scriptable
	}
}

template<typename message_type>
void pass_events_to_script(world& owner, int msg_enum) {
	auto& events = owner.get_message_queue<message_type>();

	if (std::string(typeid(message_type).name()) == "struct messages::damage_message") {
		int breakp = 123123;
	}

	events.erase(
		std::remove_if(events.begin(), events.end(), [msg_enum](message_type& msg){
			auto* scriptable = msg.subject->find<components::scriptable>();
			if (scriptable == nullptr || !scriptable->available_scripts) return false;

			auto it = scriptable->available_scripts->get_raw().find(msg_enum);

			if (it != scriptable->available_scripts->get_raw().end()) {
				return !luabind::call_function<bool>((*it).second, boost::ref(msg));
			}
			
			return false;
	}), events.end());
}
using namespace messages;
void script_system::process_events(world& owner) {
	try {
		pass_events_to_script<collision_message>(owner, components::scriptable::COLLISION_MESSAGE);
		pass_events_to_script<damage_message>(owner, components::scriptable::DAMAGE_MESSAGE);
		pass_events_to_script<intent_message>(owner, components::scriptable::INTENT_MESSAGE);
	}
	catch (std::exception compilation_error) {
		std::cout << compilation_error.what() << std::endl;
	}
}