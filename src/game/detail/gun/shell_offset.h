#pragma once
#include "game/detail/gun/is_geometry_flipped.h"

template <class T>
transformi calc_shell_offset(const T& gun_handle) {
	if (const auto sprite = gun_handle.template find<invariants::sprite>()) {
		const auto offsets = gun_handle.get_cosmos().get_logical_assets().get_offsets(sprite->image_id);

		auto offset = offsets.gun.shell_spawn;

		if (::is_geometry_flipped(gun_handle)) {
			offset.flip_vertically();
		}

		return offset;
	}

	return {};
}

