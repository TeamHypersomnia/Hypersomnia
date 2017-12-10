#pragma once
#include "augs/misc/constant_size_vector.h"
#include "augs/misc/enum/enum_array.h"

#include "game/container_sizes.h"
#include "game/transcendental/component_synchronizer.h"

namespace components {
	struct shape_circle {
		static constexpr bool is_synchronized = true;

		// GEN INTROSPECTOR struct components::shape_circle
		real32 radius = 0.f;
		bool activated = true;
		pad_bytes<3> pad;
		// END GEN INTROSPECTOR
	};
}

template<bool is_const>
class basic_shape_circle_synchronizer : public component_synchronizer_base<is_const, components::shape_circle> {
protected:
	using base = component_synchronizer_base<is_const, components::shape_circle>;
	using base::handle;
public:
	using base::component_synchronizer_base;
	using base::get_raw_component;

	real32 get_radius() const;
	bool is_activated() const;
};

template<>
class component_synchronizer<false, components::shape_circle> : public basic_shape_circle_synchronizer<false> {
	void reinfer_caches() const;
public:
	using basic_shape_circle_synchronizer<false>::basic_shape_circle_synchronizer;

	void set_activated(const bool flag) const;
};

template<>
class component_synchronizer<true, components::shape_circle> : public basic_shape_circle_synchronizer<true> {
public:
	using basic_shape_circle_synchronizer<true>::basic_shape_circle_synchronizer;
};