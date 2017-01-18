#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "augs/graphics/renderable_positioning_type.h"
#include "augs/math/rects.h"
#include "augs/build_settings/setting_empty_bases.h"

class interpolation_system;

template<bool is_const, class entity_handle_type>
class basic_renderable_mixin {
public:
	ltrb get_aabb(const renderable_positioning_type type = renderable_positioning_type::CENTER) const;
	ltrb get_aabb(const components::transform transform, const renderable_positioning_type type = renderable_positioning_type::CENTER) const;
	ltrb get_aabb(const interpolation_system&, const renderable_positioning_type type = renderable_positioning_type::CENTER) const;
};

template<bool, class>
class renderable_mixin;

template<class entity_handle_type>
class EMPTY_BASES renderable_mixin<false, entity_handle_type> : public basic_renderable_mixin<false, entity_handle_type> {
public:
};

template<class entity_handle_type>
class EMPTY_BASES renderable_mixin<true, entity_handle_type> : public basic_renderable_mixin<true, entity_handle_type> {
};