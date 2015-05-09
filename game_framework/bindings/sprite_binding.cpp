#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../resources/render_info.h"
#include "../game/texture_helper.h"
#include "../systems/render_system.h"

#include "misc/vector_wrapper.h"

namespace bindings {
	luabind::scope _sprite() {
		return
			luabind::class_<renderable>("renderable"),

			luabind::class_<renderable::draw_input>("draw_input")
			.def(luabind::constructor<>())
			.def(luabind::constructor<const renderable::draw_input&>())
			.def_readwrite("camera_transform", &renderable::draw_input::camera_transform)
			.def_readwrite("visible_area", &renderable::draw_input::visible_area)
			.def_readwrite("additional_info", &renderable::draw_input::additional_info)
			.def_readwrite("output", &renderable::draw_input::output)
			.def_readwrite("transform", &renderable::draw_input::transform)
			.def_readwrite("always_visible", &renderable::draw_input::always_visible)
			.def_readwrite("rotated_camera_aabb", &renderable::draw_input::rotated_camera_aabb)
			,

			luabind::class_<sprite, renderable>("sprite")
			.def(luabind::constructor<>())
			.def_readwrite("size", &sprite::size)
			.def_readwrite("image", &sprite::tex)
			.def_readwrite("rotation_offset", &sprite::rotation_offset)
			.def_readwrite("color", &sprite::color)
			.def("update_size", &sprite::update_size)
			.def("draw", &sprite::draw),
			

			luabind::class_<tileset::tile_type>("tile_type")
			.def(luabind::constructor<texture_baker::texture*>())
			.def_readwrite("tile_texture", &tileset::tile_type::tile_texture),

			augs::misc::vector_wrapper<tileset::tile_type>::bind_vector("tile_type_vector"),

			luabind::class_<tileset>("tileset")
			.def(luabind::constructor<>())
			.def_readwrite("tile_types", &tileset::tile_types),
			
			luabind::class_<tile_layer::tile>("tile_object")
			.def(luabind::constructor<unsigned>())
			.def_readwrite("type", &tile_layer::tile::type_id)
			,

			augs::misc::vector_wrapper<tile_layer::tile>::bind_vector("tile_vector"),

			luabind::class_<tile_layer, renderable>("tile_layer")
			.def(luabind::constructor<rects::wh<int>>())
			.def("generate_indices_by_type", &tile_layer::generate_indices_by_type)
			.def_readwrite("size", &tile_layer::size)
			.def_readwrite("tiles", &tile_layer::tiles)
			.def_readwrite("layer_tileset", &tile_layer::layer_tileset)
			.def_readwrite("square_size", &tile_layer::square_size)
			;
	}
}