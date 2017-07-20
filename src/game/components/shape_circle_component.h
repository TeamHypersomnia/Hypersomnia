#pragma once
#include "augs/misc/constant_size_vector.h"
#include "augs/misc/enum_array.h"
#include "game/detail/convex_partitioned_shape.h"
#include "game/container_sizes.h"

#include "game/transcendental/component_synchronizer.h"

namespace components {
	struct shape_circle {
		static constexpr bool is_synchronized = true;

		// GEN INTROSPECTOR struct components::shape_circle
		float radius = 0.f;
		bool activated = true;
		pad_bytes<3> pad;
		// END GEN INTROSPECTOR
	};
}

template<bool is_const>
class basic_shape_circle_synchronizer : public component_synchronizer_base<is_const, components::shape_circle> {
public:
	using component_synchronizer_base<is_const, components::shape_circle>::component_synchronizer_base;

	auto get_radius() const {
		return get_raw_component().radius;
	}

	bool is_activated() const {
		return get_raw_component().activated;
	}
};

template<>
class component_synchronizer<false, components::shape_circle> : public basic_shape_circle_synchronizer<false> {
	void reinference() const {
		handle.get_cosmos().partial_reinference<physics_system>(handle);
	}
public:
	using basic_shape_circle_synchronizer<false>::basic_shape_circle_synchronizer;

	void set_activated(const bool flag) const {
		if (flag == get_raw_component().activated) {
			return;
		}

		get_raw_component().activated = flag;
		reinference();
	}
};

template<>
class component_synchronizer<true, components::shape_circle> : public basic_shape_circle_synchronizer<true> {
public:
	using basic_shape_circle_synchronizer<true>::basic_shape_circle_synchronizer;
};