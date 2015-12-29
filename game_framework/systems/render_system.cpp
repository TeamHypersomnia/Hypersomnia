#include <algorithm>

#include "render_system.h"
#include "entity_system/entity.h"
#include "../resources/render_info.h"

#include "utilities/entity_system/overworld.h"


void render_system::generate_layers(int mask) {
	layers.clear();

	/* shortcut */
	std::vector<entity_id> entities_by_mask;
	for (auto it : targets) {
		if (it->get<components::render>().mask == mask)
			entities_by_mask.push_back(it);
	}

	for (auto it : entities_by_mask) {
		auto layer = it->get<components::render>().layer;
		
		if (layer >= layers.size()) 
			layers.resize(layer+1);

		layers[layer].push_back(it);
	}
}

void render_system::set_current_transforms_as_previous_for_interpolation() {
	for (auto it : targets) {
		auto& render = it->get<components::render>();

		if (render.interpolate) {
			render.previous_transform = it->get<components::transform>();
		}
	}
}

void render_system::draw_layer(resources::renderable::drawing_state& in, int layer) {
	auto ratio = view_interpolation_ratio();

	auto in_camera_transform = in.camera_transform;
	auto in_always_visible = in.always_visible;
	
	if (layer < layers.size() && !layers[layer].empty()) {
		for (auto e : layers[layer]) {
			auto& render = e->get<components::render>();
			if (render.model == nullptr) continue;

			if (render.interpolate) {
				auto& actual_transform = e->get<components::transform>();

				components::transform interpolated_transform;
				interpolated_transform.pos = actual_transform.pos * ratio + render.previous_transform.pos * (1.0f - ratio);
				interpolated_transform.rotation = actual_transform.rotation * ratio + render.previous_transform.rotation * (1.0f - ratio);

				in.transform = interpolated_transform;
			}
			else
				in.transform = e->get<components::transform>();
			
			in.additional_info = &render;

			in.camera_transform = render.absolute_transform ? components::transform() : in_camera_transform;
			in.always_visible = render.absolute_transform ? true : in_always_visible;

			render.model->draw(in);
		}
	}

	in.camera_transform = in_camera_transform;
	in.always_visible = in_always_visible;
}

void render_system::generate_and_draw_all_layers(resources::renderable::drawing_state& in, int mask) {
	generate_layers(mask);

	for (size_t i = 0; i < layers.size(); ++i)
		draw_layer(in, layers.size()-i-1);
}

