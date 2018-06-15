#pragma once
#include <algorithm>
#include "augs/math/transform.h"
#include "augs/math/rects.h"
#include "augs/math/matrix.h"

template <class T>
struct basic_camera_cone {
	using vec2_type = basic_vec2<T>;
	using ltrb = basic_ltrb<T>;

	// GEN INTROSPECTOR struct basic_camera_cone class T
	basic_transform<T> transform;
	T zoom;
	// END GEN INTROSPECTOR

	basic_camera_cone(
		const basic_transform<T>& transform = basic_transform<T>(),
		const T zoom = 1.f
	) : 
		transform(transform),
		zoom(zoom)
	{}

	vec2_type to_screen_space(const vec2_type screen_size, const vec2_type world_pos) const {
		// TODO: support rotation
		return zoom * (world_pos - transform.pos) + screen_size / 2;
	}

	ltrb to_screen_space(const vec2_type screen_size, const ltrb r) const {
		// TODO: support rotation
		return { to_screen_space(screen_size, r.get_position()), r.get_size() * zoom };
	}

	auto get_visible_world_area(const vec2_type screen_size) const {
		return screen_size / zoom;
	}

	ltrb get_visible_world_rect_aabb(const vec2_type screen_size) const {
		const auto visible_world_area = get_visible_world_area(screen_size);
		return augs::get_aabb_rotated(visible_world_area, transform.rotation) + transform.pos - visible_world_area / 2;
	}
	
	auto get_projection_matrix(const vec2_type screen_size) const {
		// TODO: support rotation
		return augs::orthographic_projection(get_visible_world_rect_aabb(screen_size));
	}

	vec2_type to_world_space(
		const vec2_type screen_size,
		const vec2_type cursor_pos
	) const {
		// TODO: support rotation
		return transform.pos + (cursor_pos - screen_size / 2) / zoom;
	}
};

using camera_cone = basic_camera_cone<float>;