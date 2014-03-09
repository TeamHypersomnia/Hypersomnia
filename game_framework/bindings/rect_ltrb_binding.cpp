#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "math/rects.h"

namespace bindings {
	luabind::scope _rect_ltrb() {
		return
			luabind::class_<rects::ltrb<float>>("rect_ltrb")
			.def(luabind::constructor<const rects::xywh<float>&>())
			.def(luabind::constructor<float, float, float, float>())
			.def_readwrite("l", &rects::ltrb<float>::l)
			.def_readwrite("t", &rects::ltrb<float>::t)
			.def_readwrite("r", &rects::ltrb<float>::r)
			.def_readwrite("b", &rects::ltrb<float>::b)
			.property("w", (float (rects::ltrb<float>::*)() const)&rects::ltrb<float>::w, (void (rects::ltrb<float>::*)(float)) &rects::ltrb<float>::w)
			.property("h", (float (rects::ltrb<float>::*)() const)&rects::ltrb<float>::h, (void (rects::ltrb<float>::*)(float)) &rects::ltrb<float>::h);
	}
}