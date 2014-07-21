#pragma once
#include "stdafx.h"
#include "bindings.h"
#include <random>

unsigned std_random_device() {
	return std::random_device()();
}

namespace bindings {
	luabind::scope _random_binding() {
		return
			luabind::class_<std::mt19937>("std_mt19937")
			.def(luabind::constructor<>())
			.def(luabind::constructor<unsigned>())
			.def("seed", &std::mt19937::seed),

			luabind::def("std_random_device", std_random_device),

			luabind::def("randval", (float(*)(std::pair<float, float>))&randval),
			luabind::def("randval", (float(*)(float, float))&randval),
			luabind::def("randval_i", (int(*)(int, int))&randval),

			luabind::def("randval", (float(*)(std::pair<float, float>, std::mt19937&))&randval),
			luabind::def("randval", (float(*)(float, float, std::mt19937&))&randval),
			luabind::def("randval_i", (int(*)(int, int, std::mt19937&))&randval);
	}
}