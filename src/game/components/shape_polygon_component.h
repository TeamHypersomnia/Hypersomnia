#pragma once
#include "augs/pad_bytes.h"

#include "augs/misc/constant_size_vector.h"
#include "augs/misc/enum/enum_array.h"
#include "augs/misc/convex_partitioned_shape.h"

#include "game/assets/all_logical_assets_declarations.h"
#include "game/transcendental/component_synchronizer.h"

#include "game/components/all_inferred_state_component.h"

struct b2Fixture_index_in_component;

struct convex_poly_destruction_scar {
	// GEN INTROSPECTOR struct convex_poly_destruction_scar
	vec2 first_impact;
	vec2 depth_point;
	// END GEN INTROSPECTOR
};

struct convex_poly_destruction_data {
	// GEN INTROSPECTOR struct convex_poly_destruction_data
	augs::constant_size_vector<convex_poly_destruction_scar, DESTRUCTION_SCARS_COUNT> scars;
	// END GEN INTROSPECTOR
};

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
protected:
	using base = component_synchronizer_base<is_const, components::shape_polygon>;
	using base::handle;
public:
	using base::component_synchronizer_base;
	using base::get_raw_component;

	bool is_activated() const;
};

template<>
class component_synchronizer<false, components::shape_polygon> : public basic_shape_polygon_synchronizer<false> {
	void reinference() const;
public:
	using basic_shape_polygon_synchronizer<false>::basic_shape_polygon_synchronizer;

	convex_poly_destruction_data& get_modifiable_destruction_data(
		const b2Fixture_index_in_component indices
	) const;

	void set_activated(const bool flag) const;
};

template<>
class component_synchronizer<true, components::shape_polygon> : public basic_shape_polygon_synchronizer<true> {
public:
	using basic_shape_polygon_synchronizer<true>::basic_shape_polygon_synchronizer;
};