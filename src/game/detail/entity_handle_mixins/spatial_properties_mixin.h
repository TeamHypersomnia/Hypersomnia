#pragma once
#include <optional>
#include "augs/build_settings/platform_defines.h"

#include "game/transcendental/entity_handle_declaration.h"

#include "game/components/transform_component.h"
#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/components/wandering_pixels_component.h"

struct all_logical_assets;

template <bool is_const, class entity_handle_type>
class basic_spatial_properties_mixin {
public:
	components::transform get_logic_transform() const;
	std::optional<components::transform> find_logic_transform() const;

	template <class interpolation_system_type>
	components::transform get_viewing_transform(const interpolation_system_type& sys, const bool integerize = false) const {
		const auto handle = *static_cast<const entity_handle_type*>(this);

		if (const auto owner = handle.get_owner_of_colliders();
			owner.alive() && owner != handle 
		) {
			auto body_transform = sys.get_interpolated(owner);

			if (integerize) {
				body_transform.pos.discard_fract();
			}

			const auto offset = handle.calculate_owner_of_colliders();

			auto displacement = offset.shape_offset;

			if (!displacement.pos.is_zero()) {
				displacement.pos.rotate(body_transform.rotation, vec2(0, 0));
			}

			return body_transform + displacement;
		}
		
		return sys.get_interpolated(handle);
	}
	
	vec2 get_effective_velocity() const;

	ltrb get_aabb() const {
		const auto handle = *static_cast<const entity_handle_type*>(this);
		return get_aabb(handle.get_logic_transform());
	}

	template <class interpolation_system_type>
	ltrb get_aabb(const interpolation_system_type& interp) const {
		const auto handle = *static_cast<const entity_handle_type*>(this);

		return get_aabb(handle.get_viewing_transform(interp, true));
	}

	ltrb get_aabb(const components::transform transform) const {
		const auto handle = *static_cast<const entity_handle_type*>(this);

		if (const auto* const sprite = handle.template find_def<invariants::sprite>();
			sprite != nullptr
		) {
			return sprite->get_aabb(transform);
		}

		if (const auto* const polygon = handle.template find_def<invariants::polygon>();
			polygon != nullptr
		) {
			return polygon->get_aabb(transform);
		}

		if (const auto* const wandering_pixels = handle.template find<components::wandering_pixels>();
			wandering_pixels != nullptr
		) {
			return wandering_pixels->reach;
		}

		/* TODO: Implement get_aabb for physical entities */
		ensure(!handle.template has<components::rigid_body>());
		ensure(nullptr == handle.template find_def<invariants::fixtures>());

		return {};
	}
};

template<bool, class>
class spatial_properties_mixin;

template<class entity_handle_type>
class spatial_properties_mixin<false, entity_handle_type> : public basic_spatial_properties_mixin<false, entity_handle_type> {
public:
	void set_logic_transform(
		const logic_step step,
		const components::transform
	) const;
};

template<class entity_handle_type>
class spatial_properties_mixin<true, entity_handle_type> : public basic_spatial_properties_mixin<true, entity_handle_type> {
};
