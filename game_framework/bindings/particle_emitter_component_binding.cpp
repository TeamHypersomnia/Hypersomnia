#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../resources/particle_emitter_info.h"
#include "../components/particle_emitter_component.h"

namespace bindings {
	luabind::scope _particle_emitter_component() {
		return (
			particle_emitter_info::bind("particle_emitter_info"),

			luabind::class_<components::particle_emitter>("particle_emitter_component")
			.def(luabind::constructor<>())
			.def(luabind::constructor<particle_emitter_info*>())
			.def_readwrite("available_particle_effects", &components::particle_emitter::available_particle_effects)
			.def("get_effect", &components::particle_emitter::get_effect))
			;
	}
}