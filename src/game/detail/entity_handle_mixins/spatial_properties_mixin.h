#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/components/transform_component.h"
#include "augs/build_settings/platform_defines.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/wandering_pixels_component.h"

class interpolation_system;
class all_logical_assets;

template <bool is_const, class entity_handle_type>
class basic_spatial_properties_mixin {
public:
	bool has_logic_transform() const;
	components::transform get_logic_transform() const;
	components::transform get_viewing_transform(const interpolation_system& sys, const bool integerize = false) const;
	vec2 get_effective_velocity() const;

	ltrb get_aabb() const {
		const auto handle = *static_cast<const entity_handle_type*>(this);
		return get_aabb(handle.get_logic_transform());
	}

	ltrb get_aabb(const interpolation_system& interp) const {
		const auto handle = *static_cast<const entity_handle_type*>(this);

		return get_aabb(handle.get_viewing_transform(interp, true));
	}

	ltrb get_aabb(const components::transform transform) const {
		const auto handle = *static_cast<const entity_handle_type*>(this);

		if (
			const auto* const sprite = handle.find<components::sprite>();
			sprite != nullptr
		) {
			return sprite->get_aabb(transform);
		}

		if (
			const auto* const polygon = handle.find<components::polygon>();
			polygon != nullptr
		) {
			return polygon->get_aabb(transform);
		}

		if (
			const auto* const wandering_pixels = handle.find<components::wandering_pixels>();
			wandering_pixels != nullptr
		) {
			return wandering_pixels->reach;
		}

		if (
			const auto* const particles_existence = handle.find<components::particles_existence>();
			particles_existence != nullptr
		) {
			ltrb aabb;
			aabb.set_position(transform.pos);
			aabb.set_size({ 2.f, 2.f });

			const auto enlarge = std::max(
				particles_existence->input.displace_source_position_within_radius, 
				particles_existence->distribute_within_segment_of_length
			);

			aabb.expand_from_center({ enlarge, enlarge });

			return aabb;
		}

		//const auto* const sound_existence = e.find<components::sound_existence>();
		//if (sound_existence) {
		//	result.type = tree_type::SOUND_EXISTENCES;
		//	result.aabb.set_position(e.get_logic_transform().pos);
		//
		//	const float artifacts_avoidance_epsilon = 20.f;
		//
		//	const float distance = sound_existence->calculate_max_audible_distance() + artifacts_avoidance_epsilon;
		//	result.aabb.set_size({ distance*2, distance * 2 });
		//}

		//ensure(false);

		/* TODO: Implement get_aabb for physical entities */
		ensure(!handle.has<components::rigid_body>());
		ensure(!handle.has<components::fixtures>());

		return{};
	}
};

template<bool, class>
class spatial_properties_mixin;

template<class entity_handle_type>
class EMPTY_BASES spatial_properties_mixin<false, entity_handle_type> : public basic_spatial_properties_mixin<false, entity_handle_type> {
public:
	void set_logic_transform(
		const logic_step step,
		const components::transform
	) const;
};

template<class entity_handle_type>
class EMPTY_BASES spatial_properties_mixin<true, entity_handle_type> : public basic_spatial_properties_mixin<true, entity_handle_type> {
};