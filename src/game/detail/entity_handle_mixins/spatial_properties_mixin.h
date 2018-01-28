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
	std::optional<components::transform> find_viewing_transform(const interpolation_system_type& sys, const bool integerize = false) const {
		const auto handle = *static_cast<const entity_handle_type*>(this);
		const auto& cosmos = handle.get_cosmos();

		if (const auto connection = handle.find_colliders_connection();
			connection && connection->owner != handle
		) {
			if (auto body_transform = sys.find_interpolated(cosmos[connection->owner])) {
				auto bt = *body_transform;

				if (integerize) {
					bt.pos.discard_fract();
				}

				auto displacement = connection->shape_offset;

				if (!displacement.pos.is_zero()) {
					displacement.pos.rotate(bt.rotation, vec2(0, 0));
				}

				return bt + displacement;
			}

			return std::nullopt;
		}
		
		return sys.find_interpolated(handle);
	}
	
	vec2 get_effective_velocity() const;

	std::optional<ltrb> find_aabb() const {
		const auto handle = *static_cast<const entity_handle_type*>(this);

		if (const auto transform = handle.find_logic_transform()) {
			return find_aabb(*transform);
		}

		return std::nullopt;
	}

	template <class interpolation_system_type>
	std::optional<ltrb> find_aabb(const interpolation_system_type& interp) const {
		const auto handle = *static_cast<const entity_handle_type*>(this);

		if (const auto t = handle.find_viewing_transform(interp, true)) {
			return find_aabb(*t);
		}

		return std::nullopt;
	}

	std::optional<ltrb> find_aabb(const components::transform transform) const {
		const auto handle = *static_cast<const entity_handle_type*>(this);

		if (const auto* const sprite = handle.template find<invariants::sprite>();
			sprite != nullptr
		) {
			return sprite->get_aabb(transform);
		}

		if (const auto* const polygon = handle.template find<invariants::polygon>();
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
		ensure(nullptr == handle.template find<invariants::fixtures>());

		return std::nullopt;
	}


	/* Compatibility shortcuts. Their use is not recommended henceforth. */

	template <class... Args>
	auto get_aabb(Args&&... args) const {
		return *find_aabb(std::forward<Args>(args)...);
	}

	template <class... Args>
	auto get_viewing_transform(Args&&... args) const {
		return *find_viewing_transform(std::forward<Args>(args)...);
	}
};

template<bool, class>
class spatial_properties_mixin;

template<class entity_handle_type>
class spatial_properties_mixin<false, entity_handle_type> : public basic_spatial_properties_mixin<false, entity_handle_type> {
public:
	void set_logic_transform(const components::transform) const;
};

template<class entity_handle_type>
class spatial_properties_mixin<true, entity_handle_type> : public basic_spatial_properties_mixin<true, entity_handle_type> {
};
