#pragma once
#include "augs/misc/constant_size_vector.h"
#include "augs/misc/enum_array.h"
#include "game/detail/convex_partitioned_shape.h"
#include "game/detail/physics/b2Fixture_index_in_component.h"
#include "game/container_sizes.h"

#include "game/transcendental/component_synchronizer.h"

#include "augs/pad_bytes.h"
#include "game/components/all_inferred_state_component.h"

namespace components {
	struct shape_polygon {
		static constexpr bool is_synchronized = true;

		// GEN INTROSPECTOR struct components::shape_polygon
		convex_partitioned_shape shape;
		std::array<convex_poly_destruction_data, CONVEX_POLYS_COUNT> destruction;
		bool activated = true;
		pad_bytes<3> pad;
		// END GEN INTROSPECTOR
	};
}

template<bool is_const>
class basic_shape_polygon_synchronizer : public component_synchronizer_base<is_const, components::shape_polygon> {
public:
	using component_synchronizer_base<is_const, components::shape_polygon>::component_synchronizer_base;

	bool is_activated() const {
		return get_raw_component().activated;
	}
};

template<>
class component_synchronizer<false, components::shape_polygon> : public basic_shape_polygon_synchronizer<false> {
	void reinference() const {
		handle.get_cosmos().partial_reinference<physics_system>(handle);
	}
public:
	using basic_shape_polygon_synchronizer<false>::basic_shape_polygon_synchronizer;

	convex_poly_destruction_data& get_modifiable_destruction_data(
		const b2Fixture_index_in_component indices
	) const {
		return get_raw_component().destruction[indices.convex_shape_index];
	}

	void set_activated(const bool flag) const {
		if (flag == get_raw_component().activated) {
			return;
		}

		get_raw_component().activated = flag;
		reinference();
	}
};

template<>
class component_synchronizer<true, components::shape_polygon> : public basic_shape_polygon_synchronizer<true> {
public:
	using basic_shape_polygon_synchronizer<true>::basic_shape_polygon_synchronizer;
};