#pragma once
#include "stdafx.h"
#include <luabind/operator.hpp>
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "bindings.h"

#include "game_framework/all_component_includes.h"

using namespace components;

template <class T>
T& add(entity_id* _this, const T& c) {
	return _this->get().add<T>(c);
}

template<>
components::visibility& add<components::visibility>(entity_id* _this, const components::visibility& c) {
	components::visibility& vis = _this->get().add<components::visibility>(c);
	assert(&_this->get().get<components::visibility>() == &vis);
	assert(_this->get().find<components::visibility>() == &vis);
	return vis;
}

template <class T>
void set(entity_id* _this, const T& c) {
	_this->get().set<T>(c);
}

template <class T>
T* find(entity_id* _this) {
	return _this->get().find<T>();
}

template <class T>
void remove(entity_id* _this) {
	return _this->get().remove<T>();
}

luabind::object get_script(entity_id* _this) {
	return *(luabind::object*)(_this->get().script_data);
}
void set_script(entity_id* _this, luabind::object set) {
	*(luabind::object*)(_this->get().script_data) = set;
}

world& get_owner_world(entity_id* _this) {
	return _this->get().get_owner_world();
}

std::string get_name(entity_id* _this) {
	return _this->get().name;
}

namespace bindings {
	luabind::scope _entity() {
		return
			luabind::class_<entity>("_entity")
			.def("clear", &entity::clear)
			.def_readwrite("name", &entity::name)
			.property("owner_world", &entity::get_owner_world)

			.def("remove_animate", &entity::remove<animate>)
			.def("add", &entity::add<animate>)
			.property("animate", &entity::find<animate>, &entity::set<animate>)
			.def("remove_behaviour_tree", &entity::remove<behaviour_tree>)
			.def("add", &entity::add<behaviour_tree>)
			.property("behaviour_tree", &entity::find<behaviour_tree>, &entity::set<behaviour_tree>)
			.def("remove_visibility", &entity::remove<visibility>)
			.def("add", &entity::add<visibility>)
			.property("visibility", &entity::find<visibility>, &entity::set<visibility>)
			.def("remove_pathfinding", &entity::remove<pathfinding>)
			.def("add", &entity::add<pathfinding>)
			.property("pathfinding", &entity::find<pathfinding>, &entity::set<pathfinding>)
			.def("remove_camera", &entity::remove<camera>)
			.def("add", &entity::add<camera>)
			.property("camera", &entity::find<camera>, &entity::set<camera>)
			.def("remove_chase", &entity::remove<chase>)
			.def("add", &entity::add<chase>)
			.property("chase", &entity::find<chase>, &entity::set<chase>)
			.def("remove_children", &entity::remove<children>)
			.def("add", &entity::add<children>)
			.property("children", &entity::find<children>, &entity::set<children>)
			.def("remove_crosshair", &entity::remove<crosshair>)
			.def("add", &entity::add<crosshair>)
			.property("crosshair", &entity::find<crosshair>, &entity::set<crosshair>)
			.def("remove_damage", &entity::remove<damage>)
			.def("add", &entity::add<damage>)
			.property("damage", &entity::find<damage>, &entity::set<damage>)
			.def("remove_gun", &entity::remove<gun>)
			.def("add", &entity::add<gun>)
			.property("gun", &entity::find<gun>, &entity::set<gun>)
			.def("remove_input", &entity::remove<input>)
			.def("add", &entity::add<input>)
			.property("input", &entity::find<input>, &entity::set<input>)
			.def("remove_lookat", &entity::remove<lookat>)
			.def("add", &entity::add<lookat>)
			.property("lookat", &entity::find<lookat>, &entity::set<lookat>)
			.def("remove_movement", &entity::remove<movement>)
			.def("add", &entity::add<movement>)
			.property("movement", &entity::find<movement>, &entity::set<movement>)
			.def("remove_particle_group", &entity::remove<particle_group>)
			.def("add", &entity::add<particle_group>)
			.property("particle_group", &entity::find<particle_group>, &entity::set<particle_group>)
			.def("remove_particle_emitter", &entity::remove<particle_emitter>)
			.def("add", &entity::add<particle_emitter>)
			.property("particle_emitter", &entity::find<particle_emitter>, &entity::set<particle_emitter>)
			.def("remove_physics", &entity::remove<physics>)
			.def("add", &entity::add<physics>)
			.property("physics", &entity::find<physics>, &entity::set<physics>)
			.def("remove_steering", &entity::remove<steering>)
			.def("add", &entity::add<steering>)
			.property("steering", &entity::find<steering>, &entity::set<steering>)
			.def("remove_render", &entity::remove<render>)
			.def("add", &entity::add<render>)
			.property("render", &entity::find<render>, &entity::set<render>)
			.def("remove_transform", &entity::remove<transform>)
			.def("add", &entity::add<transform>)
			.property("transform", &entity::find<transform>, &entity::set<transform>),
			

			luabind::class_<entity_id>("entity_id")
			.def(luabind::constructor<>())
			.def(luabind::constructor<const entity_id&>())
			.def(luabind::const_self == luabind::const_self)
			.def("get", (entity& (entity_id::*)())&entity_id::get)
			.def("exists", &entity_id::alive)
			.def("get_pool", &entity_id::get_pool)
			.def("set", (entity_id& (entity_id::*)(const entity_id&))&entity_id::operator=)
			.property("script", get_script, set_script)
			.property("owner_world", get_owner_world)
			.property("name", get_name)
			

			.def("remove_animate", remove<animate>)
			.def("add", add<animate>)
			.property("animate", find<animate>, set<animate>)
			.def("remove_behaviour_tree", remove<behaviour_tree>)
			.def("add", add<behaviour_tree>)
			.property("behaviour_tree", find<behaviour_tree>, set<behaviour_tree>)
			.def("remove_visibility", remove<visibility>)
			.def("add", add<visibility>)
			.property("visibility", find<visibility>, set<visibility>)
			.def("remove_pathfinding", remove<pathfinding>)
			.def("add", add<pathfinding>)
			.property("pathfinding", find<pathfinding>, set<pathfinding>)
			.def("remove_camera", remove<camera>)
			.def("add", add<camera>)
			.property("camera", find<camera>, set<camera>)
			.def("remove_chase", remove<chase>)
			.def("add", add<chase>)
			.property("chase", find<chase>, set<chase>)
			.def("remove_children", remove<children>)
			.def("add", add<children>)
			.property("children", find<children>, set<children>)
			.def("remove_crosshair", remove<crosshair>)
			.def("add", add<crosshair>)
			.property("crosshair", find<crosshair>, set<crosshair>)
			.def("remove_damage", remove<damage>)
			.def("add", add<damage>)
			.property("damage", find<damage>, set<damage>)
			.def("remove_gun", remove<gun>)
			.def("add", add<gun>)
			.property("gun", find<gun>, set<gun>)
			.def("remove_input", remove<input>)
			.def("add", add<input>)
			.property("input", find<input>, set<input>)
			.def("remove_lookat", remove<lookat>)
			.def("add", add<lookat>)
			.property("lookat", find<lookat>, set<lookat>)
			.def("remove_movement", remove<movement>)
			.def("add", add<movement>)
			.property("movement", find<movement>, set<movement>)
			.def("remove_particle_group", remove<particle_group>)
			.def("add", add<particle_group>)
			.property("particle_group", find<particle_group>, set<particle_group>)
			.def("remove_particle_emitter", remove<particle_emitter>)
			.def("add", add<particle_emitter>)
			.property("particle_emitter", find<particle_emitter>, set<particle_emitter>)
			.def("remove_physics", remove<physics>)
			.def("add", add<physics>)
			.property("physics", find<physics>, set<physics>)
			.def("remove_steering", remove<steering>)
			.def("add", add<steering>)
			.property("steering", find<steering>, set<steering>)
			.def("remove_render", remove<render>)
			.def("add", add<render>)
			.property("render", find<render>, set<render>)
			.def("remove_transform", remove<transform>)
			.def("add", add<transform>)
			.property("transform", find<transform>, set<transform>)
			;
	}
}