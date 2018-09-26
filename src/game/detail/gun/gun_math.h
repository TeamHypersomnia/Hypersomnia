#pragma once
#include "game/components/gun_component.h"
#include "game/components/sprite_component.h"

template <class T>
vec2i get_bullet_spawn_offset(const T& gun_handle) {
	const auto& cosmos = gun_handle.get_cosmos();

	if (const auto* const sprite = gun_handle.template find<invariants::sprite>()) {
		const auto reference_id = sprite->image_id;
		const auto& offsets = cosmos.get_logical_assets().get_offsets(reference_id);

		return offsets.gun.bullet_spawn;
	}

	return {};
}

template <class T>
transformr calc_muzzle_transform(
	const T& gun_handle,
	const transformr& gun_transform,
	const vec2i bullet_spawn_offset
) {
	const auto w = gun_handle.get_logical_size().x;
	return gun_transform * transformr(bullet_spawn_offset + vec2(w / 2, 0));
}

template <class T>
transformr calc_muzzle_transform(
	const T& gun_handle,
	const transformr& gun_transform
) {
	return calc_muzzle_transform(gun_handle, gun_transform, get_bullet_spawn_offset(gun_handle));
}

template <class T>
vec2 calc_barrel_center(
	const T& gun_handle,
	const transformr& gun_transform
) {
	const auto bullet_spawn_offset = get_bullet_spawn_offset(gun_handle);

	return (gun_transform * transformr(vec2(0, bullet_spawn_offset.y))).pos;
}

