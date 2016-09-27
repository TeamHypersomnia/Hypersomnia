#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/components/transform_component.h"

template<bool is_const, class entity_handle_type>
class spatial_properties_getters {
public:
	components::transform logic_transform() const;
};