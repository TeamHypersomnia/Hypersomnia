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


			luabind::class_<ltrbt<long double>>("rect_ltrb_ld")
			.def(luabind::constructor<const xywht<long double>&>())
			.def(luabind::constructor<const ltrbt<long double>&>())
			.def(luabind::constructor<long double, long double, long double, long double>())
			.def_readwrite("l", &ltrbt<long double>::l)
			.def_readwrite("t", &ltrbt<long double>::t)
			.def_readwrite("r", &ltrbt<long double>::r)
			.def_readwrite("b", &ltrbt<long double>::b)
			.def("hover", (bool (ltrbt<long double>::*)(const vec2t<long double>& m) const) &ltrbt<long double>::hover)
			.property("w", (long double (ltrbt<long double>::*)() const)&ltrbt<long double>::w, (void (ltrbt<long double>::*)(long double)) &ltrbt<long double>::w)
			.property("h", (long double (ltrbt<long double>::*)() const)&ltrbt<long double>::h, (void (ltrbt<long double>::*)(long double)) &ltrbt<long double>::h)

			;
	}
}