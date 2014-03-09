#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "math/rects.h"

namespace bindings {
	luabind::scope _rect_xywh() {
		return
			luabind::class_<rects::xywh<float>>("rect_xywh")
			.def(luabind::constructor<float, float, float, float>())
			.def(luabind::constructor<const rects::ltrb<float>&>())
			.def_readwrite("x", &rects::xywh<float>::x)
			.def_readwrite("y", &rects::xywh<float>::y)
			.def_readwrite("w", &rects::xywh<float>::w)
			.def_readwrite("h", &rects::xywh<float>::h)
			.property("r", (float (rects::xywh<float>::*)() const)&rects::xywh<float>::r, (void (rects::xywh<float>::*)(float) ) &rects::xywh<float>::r)
			.property("b", (float (rects::xywh<float>::*)() const)&rects::xywh<float>::b, (void (rects::xywh<float>::*)(float) ) &rects::xywh<float>::b);
	}
}