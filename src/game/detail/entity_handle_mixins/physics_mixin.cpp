#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "physics_mixin.h"
#include "game/assets/all_logical_assets.h"

#include "game/components/rigid_body_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/shape_polygon_component.h"
#include "game/components/shape_circle_component.h"

template <bool C, class D>
D basic_physics_mixin<C, D>::get_owner_friction_ground() const {
	const auto self = *static_cast<const D*>(this);
	return self.get_cosmos()[self.get_owner_of_colliders().get_special_physics().owner_friction_ground];
}

template <bool C, class D>
colliders_connection basic_physics_mixin<C, D>::calculate_colliders_connection() const {
	const auto self = *static_cast<const D*>(this);
	const auto& cosmos = self.get_cosmos();
	
	if (const auto overridden = self.template find<components::specific_colliders_connection>()) {
		return overridden->connection;
	}

	if (const auto item = self.template find<components::item>()) {
		if (const auto slot = cosmos[item->get_current_slot()]) {
			return slot.calculate_connection_until();
		}

		return { self, {} };
	}

	return { self, {} };
}

template <bool C, class D>
real32 basic_physics_mixin<C, D>::calculate_density(
	const colliders_connection calculated_connection,
	const invariants::fixtures& def	
) const {
	const auto self = *static_cast<const D*>(this);
	const auto& cosmos = self.get_cosmos();

	real32 density = def.density;

	if (const auto* const item = self.template find<components::item>()) {
		if (const auto slot = cosmos[item->get_current_slot()]) {
			density *= cosmos[item->get_current_slot()].calculate_density_multiplier_due_to_being_attached();
		}
	}

	const auto owner_body = cosmos[calculated_connection.owner];

	if (const auto* const driver = owner_body.template find<components::driver>()) {
		if (cosmos[driver->owned_vehicle].alive()) {
			density *= driver->density_multiplier_while_driving;
		}
	}

	return density;
}

template <bool C, class D>
std::optional<colliders_connection> basic_physics_mixin<C, D>::find_colliders_connection() const {
	const auto self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();

	if (const auto& cache = cosmos.get_solvable_inferred().physics.get_colliders_cache(self);
		cache.is_constructed()
	) {
		return cache.connection;
	}

	return std::nullopt;
}

template <bool C, class D>
D basic_physics_mixin<C, D>::get_owner_of_colliders() const {
	const auto self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();

	if (const auto connection = find_colliders_connection()) {
		return cosmos[connection->owner];
	}

	return cosmos[entity_id()];
}

template <class D>
void physics_mixin<false, D>::infer_colliders() const {
	const auto self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();

	cosmos.get_solvable_inferred({}).physics.infer_cache_for_colliders(self);
}

// explicit instantiation
template class physics_mixin<false, basic_entity_handle<false>>;
template class physics_mixin<true, basic_entity_handle<true>>;
template class basic_physics_mixin<false, basic_entity_handle<false>>;
template class basic_physics_mixin<true, basic_entity_handle<true>>;