#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../game/texture_helper.h"
#include "../systems/render_system.h"

#include "misc/vector_wrapper.h"

#include "game_framework/components/sprite_component.h"
#include "game_framework/components/tile_layer_component.h"

#include "game_framework/shared/drawing_state.h"

namespace bindings {
	luabind::scope _sprite() {
		return
			luabind::class_<drawing_state>("drawing_state")
			.def(luabind::constructor<>())
			.def(luabind::constructor<const drawing_state&>())
			.def_readwrite("camera_transform", &drawing_state::camera_transform)
			.def_readwrite("visible_area", &drawing_state::visible_area)
			.def_readwrite("subject", &drawing_state::subject)
			.def_readwrite("output", &drawing_state::output)
			.def_readwrite("drawn_transform", &drawing_state::drawn_transform)
			.def_readwrite("always_visible", &drawing_state::always_visible)
			.def_readwrite("rotated_camera_aabb", &drawing_state::rotated_camera_aabb)
			,

			luabind::class_<sprite>("sprite")
			.def(luabind::constructor<>())
			.def_readwrite("size", &sprite::size)
			.def_readwrite("image", &sprite::tex)
			.def_readwrite("rotation_offset", &sprite::rotation_offset)
			.def_readwrite("color", &sprite::color)
			.def("update_size", &sprite::update_size)
			.def("draw", &sprite::draw)
			
			,

			luabind::class_<tileset::tile_type>("tile_type")
			.def(luabind::constructor<assets::texture_id>())
			.def_readwrite("tile_texture", &tileset::tile_type::tile_texture),

			bind_stdvector<tileset::tile_type>("tile_type_vector"),

			luabind::class_<tileset>("tileset")
			.def(luabind::constructor<>())
			.def_readwrite("tile_types", &tileset::tile_types),
			
			luabind::class_<tile_layer::tile>("tile_object")
			.def(luabind::constructor<unsigned>())
			.def_readwrite("type", &tile_layer::tile::type_id)
			,

			bind_stdvector<tile_layer::tile>("tile_vector"),

			luabind::class_<tile_layer>("tile_layer")
			.def(luabind::constructor<rects::wh<int>>())
			.def("generate_indices_by_type", &tile_layer::generate_indices_by_type)
			.def_readwrite("size", &tile_layer::size)
			.def_readwrite("tiles", &tile_layer::tiles)
			.def_readwrite("layer_tileset", &tile_layer::layer_tileset)
			.def_readwrite("square_size", &tile_layer::square_size)
			;
	}
}