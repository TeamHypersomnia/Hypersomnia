#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "misc/timer.h"

namespace bindings {
	luabind::scope _timer() {
		return (
			luabind::class_<timer>("timer")
			.def(luabind::constructor<>())
			.def("extract_milliseconds", &timer::extract<std::chrono::milliseconds>)
			.def("extract_seconds", &timer::extract<std::chrono::seconds>)
			.def("extract_microseconds", &timer::extract<std::chrono::microseconds>)
			.def("extract_nanoseconds", &timer::extract<std::chrono::nanoseconds>)
			.def("get_milliseconds", &timer::get<std::chrono::milliseconds>)
			.def("get_seconds", &timer::get<std::chrono::seconds>)
			.def("get_microseconds", &timer::get<std::chrono::microseconds>)
			.def("get_nanoseconds", &timer::get<std::chrono::nanoseconds>)
			.def("reset", &timer::reset)
			);
	}
}