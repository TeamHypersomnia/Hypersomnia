#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "augs/build_settings/setting_empty_bases.h"
#include "game/transcendental/step_declaration.h"

struct fixture_group_data;

template <bool is_const, class entity_handle_type>
class basic_physics_mixin {
public:
	entity_handle_type get_owner_friction_ground() const;
	
	template <class F>
	void create_fixtures_component_from_renderable(
		const const_logic_step step,
		F callback_for_created_component
	) const {
		const auto& handle = *static_cast<const entity_handle_type*>(this);
		const auto& cosmos = step.cosm;
		const auto& metas = step.input.metas_of_assets;

		if (handle.has<components::sprite>()) {
			const auto& cosm = handle.get_cosmos();
			const auto sprite = handle.get<components::sprite>();

			const auto image_size = metas[sprite.tex].get_size();
			vec2 scale = sprite.get_size(metas) / image_size;

			components::fixtures fixtures;
			fixtures.shape = metas[sprite.tex].shape;
			fixtures.shape.scale(scale);
			
			callback_for_created_component(fixtures);
		}
		if (handle.has<components::polygon>()) {
			std::vector<vec2> input;

			const auto& poly = handle.get<components::polygon>();

			input.reserve(poly.vertices.size());

			for (const auto& v : poly.vertices) {
				input.push_back(v.pos);
			}

			components::fixtures fixtures;
			fixtures.shape.add_concave_polygon(input);
			callback_for_created_component(fixtures);
		}
	}
};

template <bool, class>
class physics_mixin;

template <class entity_handle_type>
class EMPTY_BASES physics_mixin<false, entity_handle_type> : public basic_physics_mixin<false, entity_handle_type> {
public:	
};

template <class entity_handle_type>
class EMPTY_BASES physics_mixin<true, entity_handle_type> : public basic_physics_mixin<true, entity_handle_type> {
};