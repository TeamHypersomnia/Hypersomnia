#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/components/transform_component.h"
#include "augs/build_settings/setting_empty_bases.h"

class interpolation_system;

template<bool is_const, class entity_handle_type>
class basic_spatial_properties_mixin {
public:
	bool has_logic_transform() const;
	components::transform logic_transform() const;
	components::transform viewing_transform(const interpolation_system& sys, const bool integerize = false) const;
	vec2 get_effective_velocity() const;
};

template<bool, class>
class spatial_properties_mixin;

template<class entity_handle_type>
class EMPTY_BASES spatial_properties_mixin<false, entity_handle_type> : public basic_spatial_properties_mixin<false, entity_handle_type> {
public:
	void set_logic_transform(const components::transform) const;
};

template<class entity_handle_type>
class EMPTY_BASES spatial_properties_mixin<true, entity_handle_type> : public basic_spatial_properties_mixin<true, entity_handle_type> {
};