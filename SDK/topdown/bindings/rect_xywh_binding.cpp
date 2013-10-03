#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "math/rects.h"

namespace bindings {
	luabind::scope _rect_xywh() {
		return
			luabind::class_<rects::xywh>("rect_xywh")
			.def(luabind::constructor<int, int, int, int>())
			.def_readwrite("x", &rects::xywh::x)
			.def_readwrite("y", &rects::xywh::y)
			.def_readwrite("w", &rects::xywh::w)
			.def_readwrite("h", &rects::xywh::h)
			.property("r", (int (rects::xywh::*)() const)&rects::xywh::r, (void (rects::xywh::*)(int) ) &rects::xywh::r)
			.property("b", (int (rects::xywh::*)() const)&rects::xywh::b, (void (rects::xywh::*)(int) ) &rects::xywh::b);
	}
}