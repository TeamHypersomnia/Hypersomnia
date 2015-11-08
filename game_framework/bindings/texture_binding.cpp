#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "texture_baker/texture_baker.h"
#include "texture_baker/font.h"
#include "../game/texture_helper.h"

namespace bindings {
	luabind::scope _texture() {
		return (
			luabind::class_<texture>("raw_texture"),

			luabind::class_<texture_helper>("texture")
			.def(luabind::constructor<std::wstring, atlas&>())
			.def_readwrite("tex", &texture_helper::tex)
			.property("size", &texture_helper::get_size),

			luabind::class_<atlas>("atlas")
			.def(luabind::constructor<>())
			.def("build", &atlas::default_build)
			.def("linear", &atlas::linear)
			.def("nearest", &atlas::nearest)
			.def("force_bind", &atlas::_bind)
			.def("bind", &atlas::bind),

			luabind::class_<font_file>("font_file")
			.def(luabind::constructor<>())
			.def("open", (bool (font_file::*)(const char* filename, unsigned _pt, const std::wstring& characters))&font_file::open)
			.def("free_images", &font_file::free_images)
			,

			luabind::class_<font>("font_instance")
			.def(luabind::constructor<>())
			.def("build", &font::build)
			.def("add_to_atlas", &font::add_to_atlas)

			);
	}
}