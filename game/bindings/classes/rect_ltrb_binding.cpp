#include "stdafx.h"
#include "game/bindings/bindings.h"

#include "augs/math/rects.h"
#include "augs/math/vec2.h"

namespace bindings { 
	luabind::scope _rect_ltrb() {
		return
			luabind::class_<ltrb>("rect_ltrb")
			.def(luabind::constructor<const xywh&>())
			.def(luabind::constructor<const ltrb&>())
			.def(luabind::constructor<float, float, float, float>())
			.def_readwrite("l", &ltrb::l)
			.def_readwrite("t", &ltrb::t)
			.def_readwrite("r", &ltrb::r)
			.def_readwrite("b", &ltrb::b)
			.def("hover", (bool (ltrb::*)(const vec2& m) const) &ltrb::hover)
			.property("w", (float (ltrb::*)() const)&ltrb::w, (void (ltrb::*)(float)) &ltrb::w)
			.property("h", (float (ltrb::*)() const)&ltrb::h, (void (ltrb::*)(float)) &ltrb::h),


			luabind::class_<ltrbi>("rect_ltrb_i")
			.def(luabind::constructor<const xywhi&>())
			.def(luabind::constructor<const ltrbi&>())
			.def(luabind::constructor<int, int, int, int>())
			.def_readwrite("l", &ltrbi::l)
			.def_readwrite("t", &ltrbi::t)
			.def_readwrite("r", &ltrbi::r)
			.def_readwrite("b", &ltrbi::b)
			.property("w", (int (ltrbi::*)() const)&ltrbi::w, (void (ltrbi::*)(int)) &ltrbi::w)
			.property("h", (int (ltrbi::*)() const)&ltrbi::h, (void (ltrbi::*)(int)) &ltrbi::h),


			luabind::class_<rects::ltrb<long double>>("rect_ltrb_ld")
			.def(luabind::constructor<const rects::xywh<long double>&>())
			.def(luabind::constructor<const rects::ltrb<long double>&>())
			.def(luabind::constructor<long double, long double, long double, long double>())
			.def_readwrite("l", &rects::ltrb<long double>::l)
			.def_readwrite("t", &rects::ltrb<long double>::t)
			.def_readwrite("r", &rects::ltrb<long double>::r)
			.def_readwrite("b", &rects::ltrb<long double>::b)
			.def("hover", (bool (rects::ltrb<long double>::*)(const vec2t<long double>& m) const) &rects::ltrb<long double>::hover)
			.property("w", (long double (rects::ltrb<long double>::*)() const)&rects::ltrb<long double>::w, (void (rects::ltrb<long double>::*)(long double)) &rects::ltrb<long double>::w)
			.property("h", (long double (rects::ltrb<long double>::*)() const)&rects::ltrb<long double>::h, (void (rects::ltrb<long double>::*)(long double)) &rects::ltrb<long double>::h)

			;
	}
}