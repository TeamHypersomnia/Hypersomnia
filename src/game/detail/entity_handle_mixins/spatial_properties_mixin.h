#pragma once
#include <optional>
#include "augs/drawing/make_sprite_points.h"
#include "augs/build_settings/platform_defines.h"

#include "game/cosmos/entity_handle_declaration.h"
#include "game/detail/entity_handle_mixins/get_current_slot.hpp"

#include "game/components/transform_component.h"
#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/components/wandering_pixels_component.h"
#include "game/components/item_sync.h"

template <class entity_handle_type>
class spatial_properties_mixin {
public:
	bool has_independent_transform() const;
	void set_logic_transform(const transformr t) const;

	template <class F, class... K>
	void access_independent_transform(
		F callback,
		K... keys	
	) const {
		using E = entity_handle_type;

		if constexpr(E::is_specific) {
			const auto handle = *static_cast<const entity_handle_type*>(this);
			auto& components = handle.get(keys...).components;

			if constexpr(E::template has<components::rigid_body>()) {
				if (!has_independent_transform()) {
					return;
				}

				callback(std::get<components::rigid_body>(components).physics_transforms);
			}
			else if constexpr(E::template has<components::transform>()) {
				callback(std::get<components::transform>(components));
			}
			else if constexpr(E::template has<components::position>()) {
				callback(std::get<components::position>(components));
			}
		}
		else {
			static_assert(always_false_v<E>, "Not implemented for non-specific handles.");
		}
	}

	transformr get_logic_transform() const;
	std::optional<transformr> find_logic_transform() const;

	template <class interpolation_system_type>
	std::optional<transformr> find_viewing_transform(const interpolation_system_type& sys) const {
		const auto handle = *static_cast<const entity_handle_type*>(this);
		const auto& cosmos = handle.get_cosmos();

		if (const auto connection = handle.find_colliders_connection();
			connection && connection->owner != handle.get_id()
		) {
			if (auto body_transform = sys.find_interpolated(cosmos[connection->owner])) {
				auto bt = *body_transform;

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

		if (const auto t = handle.find_viewing_transform(interp)) {
			return find_aabb(*t);
		}

		return std::nullopt;
	}

	std::optional<ltrb> find_aabb(const transformr transform) const {
		const auto handle = *static_cast<const entity_handle_type*>(this);

		if (const auto overridden_geo = handle.template find<components::overridden_geo>()) {
			const auto& s = overridden_geo.get();

			if (s.is_enabled) {
				return augs::sprite_aabb(transform, s.value);
			}
		}

		if (const auto* const sprite = handle.template find<invariants::sprite>()) {
			return sprite->get_aabb(transform);
		}

		if (const auto* const polygon = handle.template find<invariants::polygon>()) {
			return polygon->get_aabb(transform);
		}

		if (const auto rigid_body = handle.template find<components::rigid_body>()) {
			return rigid_body.find_aabb();
		}

		if (const auto* const light = handle.template find<components::light>()) {
			if (const auto* const def = handle.template find<invariants::light>()) {
				return xywh::center_and_size(
					transform.pos, 
					vec2::square(def->calc_effective_reach() * 2)
				);
			}
		}

		/* Small icons are visual representations for immaterial objects like sound sources and particle effects */

		if (const auto* const sound_def = handle.template find<invariants::continuous_sound>()) {
			return xywh::center_and_size(
				transform.pos, 
				vec2::square(100)
			);
		}

		if (const auto* const particles_def = handle.template find<invariants::continuous_particles>()) {
			return xywh::center_and_size(
				transform.pos, 
				vec2::square(100)
			);
		}

		if (const auto* const marker = handle.template find<invariants::point_marker>()) {
			return xywh::center_and_size(
				transform.pos, 
				vec2::square(100)
			);
		}

		// ensure_eq(nullptr, handle.template find<invariants::fixtures>());

		return std::nullopt;
	}


	vec2 get_logical_size() const {
		if (const auto aabb = find_aabb(transformr())) {
			return aabb->get_size();
		}

		return vec2::zero;
	}

	auto& set_logical_size(const vec2 new_size) const {
		const auto handle = *static_cast<const entity_handle_type*>(this);

		if (const auto overridden_geo = handle.template find<components::overridden_geo>()) {
			overridden_geo.set(new_size);
			return *this;
		}

		return *this;
	}

	std::optional<vec2> find_logical_tip() const {
		const auto handle = *static_cast<const entity_handle_type*>(this);

		const auto w = handle.get_logical_size().x;

		if (const auto t = handle.find_logic_transform()) {
			return t->pos + vec2::from_degrees(t->rotation) * (w / 2);
		}

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

template <class E>
transformr spatial_properties_mixin<E>::get_logic_transform() const {
	const auto t = find_logic_transform();
	ensure(t.has_value());
	return *find_logic_transform();
}

template <class E>
std::optional<transformr> spatial_properties_mixin<E>::find_logic_transform() const {
	const auto handle = *static_cast<const E*>(this);

#if MORE_LOGS
	LOG("Finding transform for %x.", handle);
#endif

	if (!handle) {
		return std::nullopt;
	}

	/*
		Since an alive owner body always implies that the entity has fixtures component,
		it is equivalent to the call of:

		const auto fixtures = owner.template find<invariants::fixtures>()

		But we would anyway need to get the owner body so we do it this way.
	*/

	if (const auto connection = handle.find_colliders_connection()) {
		const auto& cosmos = handle.get_cosmos();
		const auto owner = cosmos[connection->owner];

		auto calc_transform = [&](const auto& body) {
			const auto body_transform = body.get_transform();

			auto displacement = connection->shape_offset;

			if (!displacement.pos.is_zero()) {
				displacement.pos.rotate(body_transform.rotation);
			}

			return body_transform + displacement;
		};

		/* OPTIMIZATION: If the handle is already dispatched, check if it owns itself - which is the most probable occurence. */

		if constexpr(E::is_specific) {
			if constexpr(E::template has<components::rigid_body>()) {
				if (owner.get_id() == handle.get_id()) {
					if (const auto body = handle.template get<components::rigid_body>();
						body.is_constructed()
					) {
						return calc_transform(body);
					}
				}
			}
		}

		if (const auto body = owner.template get<components::rigid_body>();
			body.is_constructed()
		) {
			return calc_transform(body);
		}
	}

	/*
		We only ever ask for transforms of fixtures, not rigid bodies. 
		We deem it pointless to ask for a position of a rigid body without fixtures.
		We will always assume that a potential rigid body comes with a fixtures component.	
		Otherwise there is anyway pretty much no reasonable way to simulate a fixtureless body.

		Thus let's have this commented out.

		if (const auto rigid_body = owner.template find<components::rigid_body>()) {
			ensure(false);

			// Should only ever happen for (entities without fixtures, but with rigid bodies), which is almost never

			return owner.template get<components::rigid_body>().get_transform();
		}
	*/

	/* The owner might have been dead due to the item being in a backpack, for example */
	if (const auto item = handle.template find<components::item>()) {
		const auto& cosmos = handle.get_cosmos();
		return cosmos[item->get_current_slot()].get_container().find_logic_transform();
	}

	if (const auto transform = handle.template find<components::transform>()) {
		return *transform;
	}	

	if (const auto pos = handle.template find<components::position>()) {
		return transformr(*pos, 0);
	}	

	return std::nullopt;
}

template <class E>
vec2 spatial_properties_mixin<E>::get_effective_velocity() const {
	const auto handle = *static_cast<const E*>(this);

	if (const auto owner = handle.get_owner_of_colliders()) {
		return owner.template get<components::rigid_body>().get_velocity();
	}

	return {};
}

template <class E>
bool spatial_properties_mixin<E>::has_independent_transform() const {
	const auto handle = *static_cast<const E*>(this);
	const auto owner_body = handle.get_owner_of_colliders();

	if (owner_body.alive() && owner_body != handle) {
		/* 
			This body is connected to a different body, 
			therefore it has no independent transform. 
		*/
		return false;
	}

	return true;
}

template <class E>
void spatial_properties_mixin<E>::set_logic_transform(const transformr t) const {
	const auto handle = *static_cast<const E*>(this);

	if (!has_independent_transform()) {
		return;
	}
	
	if (const auto rigid_body = handle.template find<components::rigid_body>()) {
		rigid_body.set_transform(t);
	}
	else if (const auto tr = handle.template find<components::transform>()) {
		*tr = t;
		// TODO: reinfer the npo cache where necessary
	}
	else if (const auto pos = handle.template find<components::position>()) {
		*pos = t.pos;
		// TODO: reinfer the npo cache where necessary
	}
}
