#pragma once
#include "game/detail/visible_entities.h"

template <class C, class F>
void for_each_iconed_entity(
	const C& cosm, 
	const visible_entities& visible,
	F callback
) {
	visible.for_each<render_layer::LIGHTS>(cosm, [&](const auto typed_handle) {
		callback(
			typed_handle,
			assets::necessary_image_id::EDITOR_ICON_LIGHT, 
			typed_handle.get_logic_transform(),
			typed_handle.template get<components::light>().color
		);
	});

	visible.for_each<render_layer::ILLUMINATING_WANDERING_PIXELS, render_layer::DIM_WANDERING_PIXELS>(cosm, [&](const auto typed_handle) {
		callback(
			typed_handle,
			assets::necessary_image_id::EDITOR_ICON_WANDERING_PIXELS, 
			typed_handle.get_logic_transform(),
			typed_handle.template get<components::wandering_pixels>().colorize
		);
	});

	visible.for_each<render_layer::CONTINUOUS_SOUNDS>(cosm, [&](const auto typed_handle) {
		callback(
			typed_handle,
			assets::necessary_image_id::EDITOR_ICON_SOUND, 
			typed_handle.get_logic_transform(),
			white
		);
	});

	visible.for_each<render_layer::CONTINUOUS_PARTICLES>(cosm, [&](const auto typed_handle) {
		callback(
			typed_handle,
			assets::necessary_image_id::EDITOR_ICON_WANDERING_PIXELS, 
			typed_handle.get_logic_transform(),
			typed_handle.template get<invariants::continuous_particles>().effect.modifier.colorize
		);
	});
}


