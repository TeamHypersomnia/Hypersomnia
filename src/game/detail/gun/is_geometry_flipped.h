#pragma once

template <class T>
bool is_geometry_flipped(const T& gun_handle) {
	if (const auto cache = ::find_colliders_cache(gun_handle)) {
		return cache->connection.flip_geometry;
	}

	return false;
}

