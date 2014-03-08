#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../resources/render_info.h"
#include "../game/texture_helper.h"
#include "../systems/render_system.h"

namespace bindings {
	luabind::scope _sprite() {
		return
			luabind::class_<renderable>("renderable"),

			luabind::class_<renderable::draw_input>("draw_input")
			.def(luabind::constructor<>())
			.def_readwrite("camera_transform", &renderable::draw_input::camera_transform)
			.def_readwrite("visible_area", &renderable::draw_input::visible_area)
			.def_readwrite("additional_info", &renderable::draw_input::additional_info)
			.def_readwrite("output", &renderable::draw_input::output)
			.def_readwrite("transform", &renderable::draw_input::transform)
			,

			luabind::class_<sprite, renderable>("sprite")
			.def(luabind::constructor<>())
			.def_readwrite("size", &sprite::size)
			.def_readwrite("image", &sprite::tex)
			.def_readwrite("rotation_offset", &sprite::rotation_offset)
			.def_readwrite("color", &sprite::color)
			.def("update_size", &sprite::update_size)
			.def("draw", &sprite::draw)
			
			;
	}
}