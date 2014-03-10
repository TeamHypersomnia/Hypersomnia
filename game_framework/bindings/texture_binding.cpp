#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "texture_baker/texture_baker.h"
#include "texture_baker/font.h"
#include "../game/texture_helper.h"

namespace bindings {
	luabind::scope _texture() {
		return (
			luabind::class_<texture_baker::texture>("raw_texture"),

			luabind::class_<texture_helper>("texture")
			.def(luabind::constructor<std::wstring, texture_baker::atlas&>())
			.def_readwrite("tex", &texture_helper::tex)
			.property("size", &texture_helper::get_size),

			luabind::class_<texture_baker::atlas>("atlas")
			.def(luabind::constructor<>())
			.def("build", &texture_baker::atlas::default_build)
			.def("linear", &texture_baker::atlas::linear)
			.def("nearest", &texture_baker::atlas::nearest)
			//.def("bind", &texture_baker::atlas::bind)
			.def("bind", &texture_baker::atlas::bind),

			luabind::class_<texture_baker::font_file>("font_file")
			.def(luabind::constructor<>())
			.def("open", (bool (texture_baker::font_file::*)(const char* filename, unsigned _pt, const std::wstring& characters))&texture_baker::font_file::open)
			.def("free_images", &texture_baker::font_file::free_images)
			,

			luabind::class_<texture_baker::font>("font_instance")
			.def(luabind::constructor<>())
			.def("build", &texture_baker::font::build)
			.def("add_to_atlas", &texture_baker::font::add_to_atlas)

			);
	}
}