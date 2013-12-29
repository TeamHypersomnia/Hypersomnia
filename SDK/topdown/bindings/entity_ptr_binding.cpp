#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "entity_system/entity_ptr.h"
#include "entity_system/entity.h"
#include <luabind/operator.hpp>

entity* dummy_pointer_instance;
namespace bindings {
	luabind::scope _entity_ptr() {
		return
			luabind::class_<entity_ptr>("entity_ptr")
			.def(luabind::constructor<>())
			.def(luabind::constructor<entity_system::entity*>())
			.def("get", &entity_ptr::get)
			.def("exists", &entity_ptr::exists)
			.def("set", &entity_ptr::set);
	}
}