#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../resources/particle_emitter_info.h"

namespace bindings {
	luabind::scope _emission() {
		return
			luabind::class_<emission>("emission")
			.def(luabind::constructor<>())
			.def_readwrite("type", &emission::type)
			.def_readwrite("spread_degrees", &emission::spread_degrees)
			.def_readwrite("velocity", &emission::velocity)
			.def_readwrite("angular_velocity", &emission::angular_velocity)
			.def_readwrite("particles_per_sec", &emission::particles_per_sec)
			.def_readwrite("stream_duration_ms", &emission::stream_duration_ms)
			.def_readwrite("particle_lifetime_ms", &emission::particle_lifetime_ms)
			.def_readwrite("size_multiplier", &emission::size_multiplier)
			.def_readwrite("acceleration", &emission::acceleration)
			.def_readwrite("particles_per_burst", &emission::particles_per_burst)
			.def_readwrite("swing_spread", &emission::swing_spread)
			.def_readwrite("swings_per_sec", &emission::swings_per_sec)
			.def_readwrite("min_swing_spread", &emission::min_swing_spread)
			.def_readwrite("min_swings_per_sec", &emission::min_swings_per_sec)
			.def_readwrite("max_swing_spread", &emission::max_swing_spread)
			.def_readwrite("max_swings_per_sec", &emission::max_swings_per_sec)
			.def_readwrite("fade_when_ms_remaining", &emission::fade_when_ms_remaining)
			.def_readwrite("swing_spread_change_rate", &emission::swing_spread_change_rate)
			.def_readwrite("swing_speed_change_rate", &emission::swing_speed_change_rate)
			.def_readwrite("initial_rotation_variation", &emission::initial_rotation_variation)
			.def_readwrite("randomize_acceleration", &emission::randomize_acceleration)
			.def_readwrite("offset", &emission::offset)
			.def_readwrite("angular_offset", &emission::angular_offset)
			.def_readwrite("particle_render_template", &emission::particle_render_template)
			.def_readwrite("num_of_particles_to_spawn_initially", &emission::num_of_particles_to_spawn_initially)
			
			.def("add_particle_template", &emission::add_particle_template)
			.enum_("emission_type")[
				luabind::value("BURST", emission::BURST),
					luabind::value("STREAM", emission::STREAM)
			];
	}
}