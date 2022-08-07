#pragma once

struct editor_material_resource;
struct editor_sprite_resource;
struct editor_sound_resource;

/*
	Right now the only non-instantiateable resource is the physical material.
*/

template <class R>
constexpr bool can_be_instantiated_v = !std::is_same_v<R, editor_material_resource>;

template <class R>
constexpr bool is_pathed_resource_v = 
	std::is_same_v<R, editor_sprite_resource> 
	|| std::is_same_v<R, editor_sound_resource>
;
