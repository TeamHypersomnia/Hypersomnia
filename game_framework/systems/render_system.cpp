#include <algorithm>

#include "render_system.h"
#include "entity_system/entity.h"
#include "../shared/drawing_state.h"

#include "utilities/entity_system/overworld.h"

#include "game_framework/components/polygon_component.h"
#include "game_framework/components/sprite_component.h"
#include "game_framework/components/tile_layer_component.h"

#include "physics_system.h"
#include "camera_system.h"

#include "entity_system/world.h"
#include "../globals/filters.h"

using namespace shared;

void render_system::determine_visible_entities_from_camera_states() {
	auto& cameras = parent_world.get_system<camera_system>().targets;
	auto& physics = parent_world.get_system<physics_system>();

	visible_entities.clear();

	for (auto& camera : cameras) {
		auto& in = camera->get<components::camera>().how_camera_will_render;

		auto& result = physics.query_aabb_px(in.rotated_camera_aabb.left_top(), in.rotated_camera_aabb.right_bottom(), filters::renderable_query());
		visible_entities.insert(visible_entities.end(), result.entities.begin(), result.entities.end());
	}

	always_visible_entities.erase(std::remove_if(always_visible_entities.begin(), always_visible_entities.end(), [this](entity_id id) {
		if (id.alive()) {
			visible_entities.push_back(id);
			return false;
		}
		return true;
	}), always_visible_entities.end());

}

void render_system::generate_layers(shared::drawing_state& in, int mask) {
	layers.clear();

	/* shortcut */
	std::vector<entity_id> entities_by_mask;

	for (auto& it : visible_entities) {
		auto* maybe_render = it->find<components::render>();
		if (!maybe_render) continue;

		if (maybe_render->mask == mask)
			entities_by_mask.push_back(it);
	}

	for (auto& it : entities_by_mask) {
		auto layer = it->get<components::render>().layer;
		
		if (layer >= layers.size()) 
			layers.resize(layer+1);

		layers[layer].push_back(it);
	}
}

void render_system::set_visibility_persistence(entity_id id, bool flag) {
	auto& it = always_visible_entities.begin();
	for (auto& e : always_visible_entities) {
		if (e == id)
			break;
		it++;
	}

	if (flag) {
		if (it == always_visible_entities.end()) 
			always_visible_entities.push_back(id);
	}
	else {
		if (it != always_visible_entities.end()) 
			always_visible_entities.erase(it);
	}
}

void render_system::set_current_transforms_as_previous_for_interpolation() {
	if (!enable_interpolation) return;

	for (auto it : visible_entities) {
		auto& render = it->get<components::render>();

		if (render.interpolate) {
			render.previous_transform = it->get<components::transform>();
		}
	}
}
template<class T>
static inline T tabs(T _a)
{
	return _a < 0 ? -_a : _a;
}

void render_system::calculate_and_set_interpolated_transforms() {
	if (!enable_interpolation) return;
	auto ratio = view_interpolation_ratio();

	for (auto e : visible_entities) {
		auto& render = e->get<components::render>();

		if (render.interpolate) {
			auto& actual_transform = e->get<components::transform>();
			render.saved_actual_transform = actual_transform;

			components::transform interpolated_transform;
			interpolated_transform.pos = actual_transform.pos * ratio + render.previous_transform.pos * (1.0f - ratio);
			interpolated_transform.rotation = vec2().set_from_degrees(render.previous_transform.rotation).lerp(vec2().set_from_degrees(actual_transform.rotation), ratio).degrees();

			if ((actual_transform.pos - interpolated_transform.pos).length_sq() > 1.f)
				actual_transform.pos = interpolated_transform.pos;
			if (tabs(actual_transform.rotation - interpolated_transform.rotation) > 1.f)
				actual_transform.rotation = interpolated_transform.rotation;

		}
	}
}

void render_system::restore_actual_transforms() {
	if (!enable_interpolation) return;

	for (auto it : visible_entities) {
		auto& render = it->get<components::render>();

		if (render.interpolate) {
			it->get<components::transform>() = render.saved_actual_transform;
		}
	}
}

void render_system::draw_layer(drawing_state& in, int layer) {
	auto in_camera_transform = in.camera_transform;
	auto in_always_visible = in.always_visible;
	
	if (layer < layers.size() && !layers[layer].empty()) {
		for (auto e : layers[layer]) {
			auto& render = e->get<components::render>();

			in.drawn_transform = e->get<components::transform>();
			in.drawn_transform.pos = vec2i(in.drawn_transform.pos);
			in.drawn_transform.rotation = int(in.drawn_transform.rotation);

			in.subject = e;

			in.camera_transform = render.absolute_transform ? components::transform() : in_camera_transform;
			in.always_visible = render.absolute_transform ? true : in_always_visible;

			auto* polygon = e->find<components::polygon>();
			auto* sprite = e->find<components::sprite>();
			auto* tile_layer = e->find<components::tile_layer>();

			if (polygon) polygon->draw(in);
			if (sprite) sprite->draw(in);
			if (tile_layer) tile_layer->draw(in);
		}
	}

	in.camera_transform = in_camera_transform;
	in.always_visible = in_always_visible;
}

void render_system::generate_and_draw_all_layers(drawing_state& in, int mask) {
	generate_layers(in, mask);

	for (size_t i = 0; i < layers.size(); ++i)
		draw_layer(in, layers.size()-i-1);
}

