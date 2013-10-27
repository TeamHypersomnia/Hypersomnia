#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "math/rects.h"

namespace bindings {
	luabind::scope _rect_ltrb() {
		return
			luabind::class_<rects::ltrb>("rect_ltrb")
			.def(luabind::constructor<const rects::xywh&>())
			.def(luabind::constructor<int, int, int, int>())
			.def_readwrite("l", &rects::ltrb::l)
			.def_readwrite("t", &rects::ltrb::t)
			.def_readwrite("r", &rects::ltrb::r)
			.def_readwrite("b", &rects::ltrb::b)
			.property("w", (int (rects::ltrb::*)() const)&rects::ltrb::w, (void (rects::ltrb::*)(int) ) &rects::ltrb::w)
			.property("h", (int (rects::ltrb::*)() const)&rects::ltrb::h, (void (rects::ltrb::*)(int) ) &rects::ltrb::h);
	}
}