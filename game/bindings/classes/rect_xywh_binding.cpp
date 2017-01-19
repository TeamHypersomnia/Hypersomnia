#include "stdafx.h"
#include "game/bindings/bindings.h"

#include "augs/math/rects.h"
#include "augs/math/vec2.h"

namespace bindings {
	luabind::scope _rect_xywh() {
		return
			luabind::class_<xywh>("rect_xywh")
			.def(luabind::constructor<float, float, float, float>())
			.def(luabind::constructor<const ltrb&>())
			.def_readwrite("x", &xywh::x)
			.def_readwrite("y", &xywh::y)
			.def_readwrite("w", &xywh::w)
			.def_readwrite("h", &xywh::h)
			.def("hover", (bool (xywh::*)(const vec2 m) const) &xywh::hover)
			.property("r", (float (xywh::*)() const)&xywh::r, (void (xywh::*)(float)) &xywh::r)
			.property("b", (float (xywh::*)() const)&xywh::b, (void (xywh::*)(float)) &xywh::b),

			luabind::class_<xywhi>("rect_xywh_i")
			.def(luabind::constructor<int, int, int, int>())
			.def(luabind::constructor<const ltrbi&>())
			.def_readwrite("x", &xywhi::x)
			.def_readwrite("y", &xywhi::y)
			.def_readwrite("w", &xywhi::w)
			.def_readwrite("h", &xywhi::h)
			.property("r", (int (xywhi::*)() const)&xywhi::r, (void (xywhi::*)(int)) &xywhi::r)
			.property("b", (int (xywhi::*)() const)&xywhi::b, (void (xywhi::*)(int)) &xywhi::b);
			;
	}
}