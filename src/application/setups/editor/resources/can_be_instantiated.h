#pragma once

struct editor_material_resource;

/*
	Right now the only non-instantiateable resource is the physical material.
*/

template <class T>
constexpr bool can_be_instantiated_v = !std::is_same_v<T, editor_material_resource>;
