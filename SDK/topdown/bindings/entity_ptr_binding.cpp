#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "entity_system/entity_ptr.h"
#include "entity_system/entity.h"

namespace bindings {
	luabind::scope _entity_ptr() {
		return
			luabind::class_<entity_ptr>("entity_ptr")
			.def(luabind::constructor<>())
			.def(luabind::constructor<entity_system::entity*>())
			.def("get", &entity_ptr::get)
			.def("set", &entity_ptr::set);
	}
}