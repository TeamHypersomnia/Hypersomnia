#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../resources/render_info.h"
#include "../game/texture_helper.h"

namespace bindings {
	luabind::scope _sprite() {
		return
			luabind::class_<sprite>("sprite")
			.def(luabind::constructor<>())
			.def_readwrite("size", &sprite::size)
			.def_readwrite("image", &sprite::tex)
			.def_readwrite("color", &sprite::color)
			.def("update_size", &sprite::update_size);
	}
}