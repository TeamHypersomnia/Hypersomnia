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
	const transformr& gun_transform
) {
	const auto bullet_spawn_offset = get_bullet_spawn_offset(gun_handle);

	if (const auto logical_width = gun_handle.find_logical_width()) {
		return gun_transform * transformr(bullet_spawn_offset + vec2(*logical_width / 2, 0));
	}

	return {};
}

template <class T>
vec2 calc_barrel_center(
	const T& gun_handle,
	const transformr& gun_transform
) {
	const auto bullet_spawn_offset = get_bullet_spawn_offset(gun_handle);

	return (gun_transform * transformr(vec2(0, bullet_spawn_offset.y))).pos;
}

