#pragma once
#include <optional>
#include "augs/math/vec2.h"
#include "augs/templates/maybe_const.h"
#include "augs/build_settings/compiler_defines.h"
#include "game/organization/all_components_declaration.h"
#include "game/detail/physics/colliders_connection.h"
#include "game/detail/entity_handle_mixins/get_current_slot.hpp"
#include "game/components/fixtures_component.h"
#include "game/components/rigid_body_sync.h"
#include "game/inferred_caches/find_physics_cache.h"

struct rigid_body_cache_info;

template <class derived_handle_type>
class physics_mixin {
public:
	static constexpr bool is_const = is_handle_const_v<derived_handle_type>;
	using generic_handle_type = basic_entity_handle<is_const>;

	generic_handle_type get_owner_friction_ground() const;
	
	auto& get_special_physics() const {
		const auto handle = *static_cast<const derived_handle_type*>(this);
		return handle.template get<components::rigid_body>().get_special();
	}

	const colliders_connection* find_colliders_connection() const;
	colliders_connection calc_colliders_connection() const;

	/* Shortcut for getting only the entity handle without shape offset */
	generic_handle_type get_owner_of_colliders() const;

	real32 calc_density(
		const colliders_connection calculated_connection,
		const invariants::fixtures& def	
	) const;

	void infer_colliders() const;
	void infer_rigid_body() const;
	void infer_colliders_from_scratch() const;
	void infer_transform() const;
};

template <class E>
typename physics_mixin<E>::generic_handle_type physics_mixin<E>::get_owner_friction_ground() const {
	const auto self = *static_cast<const E*>(this);
#if TODO_CARS
	return self.get_cosmos()[self.get_owner_of_colliders().get_special_physics().owner_friction_ground];
#else
	return self.get_cosmos()[entity_id()];
#endif
}

template <class E>
real32 physics_mixin<E>::calc_density(
	const colliders_connection calculated_connection,
	const invariants::fixtures& def	
) const {
	const auto self = *static_cast<const E*>(this);
	const auto& cosm = self.get_cosmos();

	real32 density = def.density;

	if (const auto item = self.template find<components::item>()) {
		if (const auto slot = cosm[item->get_current_slot()]) {
			density *= cosm[item->get_current_slot()].calc_density_multiplier_due_to_being_attached();
		}
	}

	const auto owner_body = cosm[calculated_connection.owner];

	if (const auto* const driver = owner_body.template find<components::driver>()) {
		if (cosm[driver->owned_vehicle].alive()) {
			density *= driver->density_multiplier_while_driving;
		}
	}

	if (const auto* const destructible = owner_body.template find<components::destructible>()) {
		if (const auto area = destructible->texture_rect.area(); area > 0.0f) {
			density /= (area);
		}
	}

	return density;
}

template <class E>
const colliders_connection* physics_mixin<E>::find_colliders_connection() const {
	const auto& self = *static_cast<const E*>(this);
	const auto cache = find_colliders_cache(self);

	if (cache) {
		return std::addressof(cache->connection);
	}

	return nullptr;
}

template <class E>
typename physics_mixin<E>::generic_handle_type physics_mixin<E>::get_owner_of_colliders() const {
	const auto self = *static_cast<const E*>(this);
	auto& cosm = self.get_cosmos();

	if (const auto connection = find_colliders_connection()) {
		return cosm[connection->owner];
	}

	return cosm[entity_id()];
}

template <class E>
void physics_mixin<E>::infer_colliders() const {
	const auto self = *static_cast<const E*>(this);
	auto& cosm = self.get_cosmos();

	cosm.get_solvable_inferred({}).physics.infer_colliders(self);
}

template <class E>
void physics_mixin<E>::infer_rigid_body() const {
	const auto self = *static_cast<const E*>(this);
	auto& cosm = self.get_cosmos();

	cosm.get_solvable_inferred({}).physics.infer_rigid_body(self);
}

template <class E>
void physics_mixin<E>::infer_colliders_from_scratch() const {
	const auto self = *static_cast<const E*>(this);
	auto& cosm = self.get_cosmos();

	cosm.get_solvable_inferred({}).physics.infer_colliders_from_scratch(self);
}

template <class E>
void physics_mixin<E>::infer_transform() const {
	const auto self = *static_cast<const E*>(this);
	auto& cosm = self.get_cosmos();

	cosm.get_solvable_inferred({}).physics.infer_rigid_body(self);
	cosm.get_solvable_inferred({}).tree_of_npo.infer_cache_for(self);

	if (self.template has<invariants::area_marker>()) {
		cosm.get_solvable_inferred({}).organisms.recalculate_grid(self);
	}
}

