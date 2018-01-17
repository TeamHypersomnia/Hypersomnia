#pragma once
#include "augs/math/vec2.h"
#include "augs/build_settings/platform_defines.h"
#include "game/organization/all_components_declaration.h"

template <bool is_const, class entity_handle_type>
class basic_physics_mixin {
public:
	entity_handle_type get_owner_friction_ground() const;
	
	bool is_physical() const {
		const auto handle = *static_cast<const entity_handle_type*>(this);
		return handle.template has<components::fixtures>() || handle.template has<components::rigid_body>();
	}

	auto& get_special_physics() const {
		const auto handle = *static_cast<const entity_handle_type*>(this);
		return handle.template get<components::rigid_body>().get_special();
	}
};

template <bool, class>
class physics_mixin;

template <class entity_handle_type>
class physics_mixin<false, entity_handle_type> : public basic_physics_mixin<false, entity_handle_type> {
public:
	using base = basic_physics_mixin<false, entity_handle_type>;
};

template <class entity_handle_type>
class physics_mixin<true, entity_handle_type> : public basic_physics_mixin<true, entity_handle_type> {
};
