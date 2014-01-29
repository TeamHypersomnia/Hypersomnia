#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../resources/particle_emitter_info.h"

namespace bindings {
	luabind::scope _particle_effect() {
		return
			luabind::class_<resources::particle_effect>("particle_effect")
			.def(luabind::constructor<>())
			.def("add", ((void (resources::particle_effect::*)(const resources::emission&))&resources::particle_effect::push_back));
	}
}