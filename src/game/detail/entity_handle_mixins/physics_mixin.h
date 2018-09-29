#pragma once
#include <optional>
#include "augs/math/vec2.h"
#include "augs/templates/maybe_const.h"
#include "augs/build_settings/platform_defines.h"
#include "game/organization/all_components_declaration.h"
#include "game/detail/physics/colliders_connection.h"
#include "game/detail/entity_handle_mixins/get_current_slot.hpp"
#include "game/detail/entity_handle_mixins/calc_connection.hpp"
#include "game/components/fixtures_component.h"
#include "game/components/rigid_body_sync.h"

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

	std::optional<colliders_connection> find_colliders_connection() const;
	std::optional<colliders_connection> calc_colliders_connection() const;

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

	assets::physical_material_id calc_physical_material() const;
};

template <class E>
typename physics_mixin<E>::generic_handle_type physics_mixin<E>::get_owner_friction_ground() const {
	const auto self = *static_cast<const E*>(this);
	return self.get_cosmos()[self.get_owner_of_colliders().get_special_physics().owner_friction_ground];
}

template <class E>
std::optional<colliders_connection> physics_mixin<E>::calc_colliders_connection() const {
	const auto self = *static_cast<const E*>(this);
	const auto& cosm = self.get_cosmos();
	
	std::optional<colliders_connection> result;

	self.template dispatch_on_having_all<invariants::fixtures>([&cosm, &result](const auto typed_self) {
		if (const auto overridden = typed_self.template find<components::specific_colliders_connection>()) {
			result = overridden->connection;
			return;
		}

		if (const auto item = typed_self.template find<components::item>()) {
			if (const auto slot = cosm[item->get_current_slot()]) {
				if (const auto topmost_container = typed_self.calc_connection_to_topmost_container()) {
					if (auto topmost_container_connection = 
						cosm[topmost_container->owner].calc_colliders_connection()
					) {
						const auto owner = topmost_container_connection->owner;
	#if MORE_LOGS
						LOG("%x (item) owned by %x", typed_self, cosm[owner]);
	#endif
						result = colliders_connection {
							owner,
							topmost_container->shape_offset * topmost_container_connection->shape_offset
						};

						return;
					}
				}

				return;
			}
		}

		if (typed_self.template find<components::rigid_body>()) {
	#if MORE_LOGS
			LOG("%x (body) owned by itself", typed_self);
	#endif
			result = colliders_connection { typed_self, {} };
			return;
		}
	});

	return result;
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

	return density;
}

template <class E>
std::optional<colliders_connection> physics_mixin<E>::find_colliders_connection() const {
	const auto self = *static_cast<const E*>(this);
	auto& cosm = self.get_cosmos();

	if (const auto cache = cosm.get_solvable_inferred().physics.find_colliders_cache(self)) {
		return cache->connection;
	}

	return calc_colliders_connection();
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
}

template <class E>
assets::physical_material_id physics_mixin<E>::calc_physical_material() const {
	const auto self = *static_cast<const E*>(this);

	if (const auto fuse = self.template find<invariants::hand_fuse>()) {
		if (self.is_like_thrown_explosive()) {
			if (fuse->released_physical_material.is_set()) {
				return fuse->released_physical_material;
			}
		}
	}

	if (const auto fixtures = self.template find<invariants::fixtures>()) {
		return fixtures->material;
	}

	return {};
}
