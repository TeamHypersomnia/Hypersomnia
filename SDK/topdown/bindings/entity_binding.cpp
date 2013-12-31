#pragma once
#include "stdafx.h"
#include <luabind/operator.hpp>
#include "entity_system/entity.h"

#include "bindings.h"

#include "../components/behaviour_tree_component.h"
#include "../components/visibility_component.h"
#include "../components/pathfinding_component.h"
#include "../components/animate_component.h"
#include "../components/camera_component.h"
#include "../components/chase_component.h"
#include "../components/children_component.h"
#include "../components/crosshair_component.h"
#include "../components/damage_component.h"
#include "../components/gun_component.h"
#include "../components/input_component.h"
#include "../components/lookat_component.h"
#include "../components/movement_component.h"
#include "../components/particle_emitter_component.h"
#include "../components/physics_component.h"
#include "../components/scriptable_component.h"
#include "../components/steering_component.h"
#include "../components/transform_component.h"
#include "../components/render_component.h"

namespace bindings {
	luabind::scope _entity() {
		return
			luabind::class_<entity>("_entity")
			.def("clear", &entity::clear)
			.def(luabind::const_self == luabind::const_self)
			.def_readwrite("name", &entity::name)
			.def("enable", &entity::enable)
			.def("disable", &entity::disable)

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
			.def("remove_particle_emitter", &entity::remove<particle_emitter>)
			.def("add", &entity::add<particle_emitter>)
			.property("particle_emitter", &entity::find<particle_emitter>, &entity::set<particle_emitter>)
			.def("remove_physics", &entity::remove<physics>)
			.def("add", &entity::add<physics>)
			.property("physics", &entity::find<physics>, &entity::set<physics>)
			.def("remove_scriptable", &entity::remove<scriptable>)
			.def("add", &entity::add<scriptable>)
			.property("scriptable", &entity::find<scriptable>, &entity::set<scriptable>)
			.def("remove_steering", &entity::remove<steering>)
			.def("add", &entity::add<steering>)
			.property("steering", &entity::find<steering>, &entity::set<steering>)
			.def("remove_render", &entity::remove<render>)
			.def("add", &entity::add<render>)
			.property("render", &entity::find<render>, &entity::set<render>)
			.def("remove_transform", &entity::remove<transform>)
			.def("add", &entity::add<transform>)
			.property("transform", &entity::find<transform>, &entity::set<transform>);
	}
}