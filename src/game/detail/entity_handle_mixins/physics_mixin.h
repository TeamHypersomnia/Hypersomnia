#pragma once
#include "augs/math/vec2.h"
#include "augs/build_settings/platform_defines.h"
#include "game/organization/all_components_declaration.h"

template <bool is_const, class entity_handle_type>
class basic_physics_mixin {
public:
	entity_handle_type get_owner_friction_ground() const;
	
	bool is_physical() const {
		const auto& handle = *static_cast<const entity_handle_type*>(this);
		return handle.template has<components::fixtures>() || handle.template has<components::rigid_body>();
	}

	template <class step_type, class F>
	void create_shape_component_from_renderable(
		const step_type step,
		F callback_for_created_component
	) const {
		const auto& handle = *static_cast<const entity_handle_type*>(this);
		const auto& cosmos = step.cosm;
		const auto& metas = step.input.logical_assets;

		if (handle.template has<components::sprite>()) {
			const auto& cosm = handle.get_cosmos();
			const auto sprite = handle.template get<components::sprite>();

			const auto image_size = metas.at(sprite.tex).get_size();
			vec2 scale = sprite.get_size() / image_size;

			/* trick to make this type dependent */
			std::decay_t<decltype(handle.template get<components::shape_polygon>().get_raw_component())> shape_polygon;
			shape_polygon.shape = metas.at(sprite.tex).shape;
			shape_polygon.shape.scale(scale);
			
			callback_for_created_component(shape_polygon);
		}
		if (handle.template has<components::polygon>()) {
			std::vector<vec2> input;

			const auto& poly = handle.template get<components::polygon>();

			input.reserve(poly.vertices.size());

			for (const auto& v : poly.vertices) {
				input.push_back(v.pos);
			}

			/* trick to make this type dependent */
			std::decay_t<decltype(handle.template get<components::shape_polygon>().get_raw_component())> shape_polygon;
			shape_polygon.shape.add_concave_polygon(input);

			callback_for_created_component(shape_polygon);
		}
	}
};

template <bool, class>
class physics_mixin;

template <class entity_handle_type>
class physics_mixin<false, entity_handle_type> : public basic_physics_mixin<false, entity_handle_type> {
public:
	using base = basic_physics_mixin<false, entity_handle_type>;

	template <class step_type>
	void add_shape_component_from_renderable(
		const step_type step
	) const {
		base::create_shape_component_from_renderable(
			step, 
			[this](const auto& component){
				const auto& handle = *static_cast<const entity_handle_type*>(this);
				handle += component;
			}
		);
	}
};

template <class entity_handle_type>
class physics_mixin<true, entity_handle_type> : public basic_physics_mixin<true, entity_handle_type> {
};
