#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "math/rects.h"

namespace bindings {
	luabind::scope _rect_ltrb() {
		return
			luabind::class_<rects::ltrb>("rect_ltrb")
			.def(luabind::constructor<const rects::xywh&>())
			.def(luabind::constructor<float, float, float, float>())
			.def_readwrite("l", &rects::ltrb::l)
			.def_readwrite("t", &rects::ltrb::t)
			.def_readwrite("r", &rects::ltrb::r)
			.def_readwrite("b", &rects::ltrb::b)
			.property("w", (float (rects::ltrb::*)() const)&rects::ltrb::w, (void (rects::ltrb::*)(float)) &rects::ltrb::w)
			.property("h", (float (rects::ltrb::*)() const)&rects::ltrb::h, (void (rects::ltrb::*)(float)) &rects::ltrb::h);
	}
}