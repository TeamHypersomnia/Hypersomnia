#include <algorithm>

#include "render_system.h"
#include "game/transcendental/entity_id.h"
#include "game/detail/state_for_drawing_camera.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/tile_layer_component.h"
#include "game/components/particle_group_component.h"

#include "game/messages/new_entity_message.h"
#include "game/messages/queue_destruction.h"

#include "game/transcendental/cosmos.h"
#include "game/enums/filters.h"

#include "game/detail/physics_scripts.h"
#include "augs/ensure.h"

std::array<std::vector<const_entity_handle>, render_layer::LAYER_COUNT> render_system::get_visible_per_layer(std::vector<const_entity_handle> entities) const {
	std::array<std::vector<entity_id>, render_layer::LAYER_COUNT> layers;
	std::array<std::vector<const_entity_handle>, render_layer::LAYER_COUNT> output;

	if (entities.empty())
		return output;

	auto& cosmos = entities[0].get_cosmos();

	for (auto& it : entities) {
		auto layer = it.get<components::render>().layer;
		ensure(layer < static_cast<render_layer>(layers.size()));
		layers[layer].push_back(it);
	}

	const std::vector<render_layer> layers_whose_order_determines_friction = { render_layer::CAR_INTERIOR } ;

	for (auto& custom_order_layer : layers_whose_order_determines_friction) {
		if (custom_order_layer < static_cast<render_layer>(layers.size())) {
			if (layers[custom_order_layer].size() > 1) {
				std::sort(layers[custom_order_layer].begin(), layers[custom_order_layer].end(), [&cosmos](entity_id b, entity_id a) {
					return are_connected_by_friction(cosmos[a], cosmos[b]);
				});
			}
		}
	}

	for (size_t layer_idx = 0; layer_idx < layers.size(); ++layer_idx) {
		output[layer_idx] = cosmos[layers[layer_idx]];
	}

	return output;
}

void render_system::draw_entities(augs::vertex_triangle_buffer& output, std::vector<const_entity_handle> entities, state_for_drawing_camera in_camera, float interpolation_ratio, bool only_border_highlights) const {
	for (auto e : entities) {
		for_each_type<components::polygon, components::sprite, /*components::tile_layer,*/ components::particle_group>([e, interpolation_ratio, &output, &in_camera, only_border_highlights](auto T) {
			typedef decltype(T) renderable_type;

			if (e.has<renderable_type>()) {
				const auto& render = e.get<components::render>();
				auto renderable_transform = viewing_transform(e, true);
				const auto& renderable = e.get<renderable_type>();

				render_system().draw_renderable(output, renderable, renderable_transform, render, in_camera, interpolation_ratio, only_border_highlights);
			}
		});
	}
}