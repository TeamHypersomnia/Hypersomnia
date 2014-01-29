#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "math/rects.h"

namespace bindings {
	luabind::scope _rect_xywh() {
		return
			luabind::class_<rects::xywh>("rect_xywh")
			.def(luabind::constructor<float, float, float, float>())
			.def(luabind::constructor<const rects::ltrb&>())
			.def_readwrite("x", &rects::xywh::x)
			.def_readwrite("y", &rects::xywh::y)
			.def_readwrite("w", &rects::xywh::w)
			.def_readwrite("h", &rects::xywh::h)
			.property("r", (float (rects::xywh::*)() const)&rects::xywh::r, (void (rects::xywh::*)(float) ) &rects::xywh::r)
			.property("b", (float (rects::xywh::*)() const)&rects::xywh::b, (void (rects::xywh::*)(float) ) &rects::xywh::b);
	}
}