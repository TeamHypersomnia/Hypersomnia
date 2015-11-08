#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "math/rects.h"
#include "math/vec2.h"

namespace bindings {
	luabind::scope _rect_xywh() {
		return
			luabind::class_<rects::wh<float>>("rect_wh")
			.def(luabind::constructor<float, float>())
			.def(luabind::constructor<const rects::wh<float>&>())
			.def_readwrite("w", &rects::xywh<float>::w)
			.def_readwrite("h", &rects::xywh<float>::h)
			,

			luabind::class_<rects::wh<int>>("rect_wh_i")
			.def(luabind::constructor<int, int>())
			.def(luabind::constructor<const rects::wh<int>&>())
			.def_readwrite("w", &rects::xywh<int>::w)
			.def_readwrite("h", &rects::xywh<int>::h)
			,

			luabind::class_<rects::xywh<float>>("rect_xywh")
			.def(luabind::constructor<float, float, float, float>())
			.def(luabind::constructor<const rects::ltrb<float>&>())
			.def_readwrite("x", &rects::xywh<float>::x)
			.def_readwrite("y", &rects::xywh<float>::y)
			.def_readwrite("w", &rects::xywh<float>::w)
			.def_readwrite("h", &rects::xywh<float>::h)
			.def("hover", (bool (rects::xywh<float>::*)(const vec2<float>& m)) &rects::xywh<float>::hover)
			.property("r", (float (rects::xywh<float>::*)() const)&rects::xywh<float>::r, (void (rects::xywh<float>::*)(float)) &rects::xywh<float>::r)
			.property("b", (float (rects::xywh<float>::*)() const)&rects::xywh<float>::b, (void (rects::xywh<float>::*)(float)) &rects::xywh<float>::b),

			luabind::class_<rects::xywh<int>>("rect_xywh_i")
			.def(luabind::constructor<int, int, int, int>())
			.def(luabind::constructor<const rects::ltrb<int>&>())
			.def_readwrite("x", &rects::xywh<int>::x)
			.def_readwrite("y", &rects::xywh<int>::y)
			.def_readwrite("w", &rects::xywh<int>::w)
			.def_readwrite("h", &rects::xywh<int>::h)
			.property("r", (int (rects::xywh<int>::*)() const)&rects::xywh<int>::r, (void (rects::xywh<int>::*)(int)) &rects::xywh<int>::r)
			.property("b", (int (rects::xywh<int>::*)() const)&rects::xywh<int>::b, (void (rects::xywh<int>::*)(int)) &rects::xywh<int>::b);

			
			;
	}
}