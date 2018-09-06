#pragma once

template <class T>
transformi calc_shell_offset(const T& gun_handle) {
	if (const auto sprite = gun_handle.template find<invariants::sprite>()) {
		const auto offsets = gun_handle.get_cosmos().get_logical_assets().get_offsets(sprite->image_id);
		return offsets.gun.shell_spawn;
	}

	return {};
}

