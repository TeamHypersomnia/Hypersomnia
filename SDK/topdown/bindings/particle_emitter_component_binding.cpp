#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../resources/particle_emitter_info.h"
#include "../components/particle_emitter_component.h"

namespace bindings {
	luabind::scope _particle_emitter_component() {
		return (
			luabind::class_<particle_emitter_info>("particle_emitter_info")
			.def(luabind::constructor<>())
			.def("add", &particle_emitter_info::add),

			luabind::class_<components::particle_emitter>("particle_emitter_component")
			.def(luabind::constructor<>())
			.def(luabind::constructor<particle_emitter_info*>())
			.def_readwrite("available_particle_effects", &components::particle_emitter::available_particle_effects));
	}
}