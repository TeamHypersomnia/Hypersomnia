#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "math/rects.h"

namespace bindings { 
	luabind::scope _rect_ltrb() {
		return
			luabind::class_<rects::ltrb<float>>("rect_ltrb")
			.def(luabind::constructor<const rects::xywh<float>&>())
			.def(luabind::constructor<const rects::ltrb<float>&>())
			.def(luabind::constructor<float, float, float, float>())
			.def_readwrite("l", &rects::ltrb<float>::l)
			.def_readwrite("t", &rects::ltrb<float>::t)
			.def_readwrite("r", &rects::ltrb<float>::r)
			.def_readwrite("b", &rects::ltrb<float>::b)
			.def("hover", (bool (rects::ltrb<float>::*)(const vec2<float>& m) const) &rects::ltrb<float>::hover)
			.property("w", (float (rects::ltrb<float>::*)() const)&rects::ltrb<float>::w, (void (rects::ltrb<float>::*)(float)) &rects::ltrb<float>::w)
			.property("h", (float (rects::ltrb<float>::*)() const)&rects::ltrb<float>::h, (void (rects::ltrb<float>::*)(float)) &rects::ltrb<float>::h),


			luabind::class_<rects::ltrb<int>>("rect_ltrb_i")
			.def(luabind::constructor<const rects::xywh<int>&>())
			.def(luabind::constructor<const rects::ltrb<int>&>())
			.def(luabind::constructor<int, int, int, int>())
			.def_readwrite("l", &rects::ltrb<int>::l)
			.def_readwrite("t", &rects::ltrb<int>::t)
			.def_readwrite("r", &rects::ltrb<int>::r)
			.def_readwrite("b", &rects::ltrb<int>::b)
			.property("w", (int (rects::ltrb<int>::*)() const)&rects::ltrb<int>::w, (void (rects::ltrb<int>::*)(int)) &rects::ltrb<int>::w)
			.property("h", (int (rects::ltrb<int>::*)() const)&rects::ltrb<int>::h, (void (rects::ltrb<int>::*)(int)) &rects::ltrb<int>::h),


			luabind::class_<rects::ltrb<long double>>("rect_ltrb_ld")
			.def(luabind::constructor<const rects::xywh<long double>&>())
			.def(luabind::constructor<const rects::ltrb<long double>&>())
			.def(luabind::constructor<long double, long double, long double, long double>())
			.def_readwrite("l", &rects::ltrb<long double>::l)
			.def_readwrite("t", &rects::ltrb<long double>::t)
			.def_readwrite("r", &rects::ltrb<long double>::r)
			.def_readwrite("b", &rects::ltrb<long double>::b)
			.def("hover", (bool (rects::ltrb<long double>::*)(const vec2<long double>& m) const) &rects::ltrb<long double>::hover)
			.property("w", (long double (rects::ltrb<long double>::*)() const)&rects::ltrb<long double>::w, (void (rects::ltrb<long double>::*)(long double)) &rects::ltrb<long double>::w)
			.property("h", (long double (rects::ltrb<long double>::*)() const)&rects::ltrb<long double>::h, (void (rects::ltrb<long double>::*)(long double)) &rects::ltrb<long double>::h)

			;
	}
}