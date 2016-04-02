#include <algorithm>

#include "render_system.h"
#include "entity_system/entity.h"
#include "../detail/state_for_drawing.h"

#include "augs/entity_system/overworld.h"

#include "game_framework/components/polygon_component.h"
#include "game_framework/components/sprite_component.h"
#include "game_framework/components/fixtures_component.h"
#include "game_framework/components/tile_layer_component.h"
#include "game_framework/components/physics_component.h"

#include "game_framework/messages/new_entity_message.h"
#include "game_framework/messages/destroy_message.h"

#include "physics_system.h"
#include "camera_system.h"

#include "entity_system/world.h"
#include "../globals/filters.h"

#include <algorithm>

using namespace shared;

render_system::render_system(world& parent_world) : event_only_system(parent_world) {
	layers_with_custom_drawing_order.push_back(render_layer::CAR_INTERIOR);
}

void render_system::add_entities_to_rendering_tree() {
	auto& events = parent_world.get_message_queue<messages::new_entity_message>();

	for (auto& it : events) {
		auto& e = it.subject;

		auto* physics_definition = e->find<components::physics_definition>();
		auto* render = e->find<components::render>();

		if (!physics_definition && render) {
			auto* sprite = e->find<components::sprite>();
			auto* polygon = e->find<components::polygon>();
			auto* tile_layer = e->find<components::tile_layer>();
			auto* particle_group = e->find<components::particle_group>();

			auto transform = e->get<components::transform>();

			render->previous_transform = transform;

			rects::ltrb<float> aabb;

			if (sprite) aabb = sprite->get_aabb(transform);
			if (polygon) aabb = polygon->get_aabb(transform);
			if (tile_layer) aabb = tile_layer->get_aabb(transform);
			// if (particle_group) aabb = particle_group->get_aabb(transform);

			b2AABB input;
			input.lowerBound = aabb.left_top();
			input.upperBound = aabb.right_bottom();

			render->rendering_proxy = non_physical_objects_tree.CreateProxy(input, new entity_id(e));
		}
	}
}

void render_system::remove_entities_from_rendering_tree() {
	auto& events = parent_world.get_message_queue<messages::destroy_message>();

	for (auto& it : events) {
		auto& e = it.subject;

		auto* physics_definition = e->find<components::physics_definition>();
		auto* render = e->find<components::render>();

		if (render && render->rendering_proxy >= 0) {
			auto* userdata = non_physical_objects_tree.GetUserData(render->rendering_proxy);
			
			if (userdata) {
				delete ((entity_id*)userdata);
				non_physical_objects_tree.DestroyProxy(render->rendering_proxy);
			}
		}
	}
}

void render_system::determine_visible_entities_from_every_camera() {
	auto& cameras = parent_world.get_system<camera_system>().targets;
	auto& physics = parent_world.get_system<physics_system>();

	visible_entities.clear();

	for (auto& camera : cameras) {
		auto& in = camera->get<components::camera>().how_camera_will_render;

		auto& result = physics.query_aabb_px(in.transformed_visible_world_area_aabb.left_top(), in.transformed_visible_world_area_aabb.right_bottom(), filters::renderable_query());
		visible_entities.insert(visible_entities.end(), result.entities.begin(), result.entities.end());

		struct render_listener {
			b2DynamicTree* tree;
			std::vector<entity_id>* visible_entities;
			bool QueryCallback(int32 node) {
				visible_entities->push_back(*((entity_id*)tree->GetUserData(node)));
				return true;
			}
		};
		
		render_listener aabb_listener;

		aabb_listener.tree = &non_physical_objects_tree;
		aabb_listener.visible_entities = &visible_entities;

		b2AABB input;
		input.lowerBound = in.transformed_visible_world_area_aabb.left_top() - vec2(400, 400);
		input.upperBound = in.transformed_visible_world_area_aabb.right_bottom() + vec2(400, 400);

		non_physical_objects_tree.Query(&aabb_listener, input);
	}

	always_visible_entities.erase(std::remove_if(always_visible_entities.begin(), always_visible_entities.end(), [this](entity_id id) {
		if (id.alive()) {
			visible_entities.push_back(id);
			return false;
		}
		return true;
	}), always_visible_entities.end());
}

void render_system::generate_layers_from_visible_entities(int mask) {
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

	for (auto& sortable_layer : layers_with_custom_drawing_order) {
		if (sortable_layer < layers.size()) {
			if (layers[sortable_layer].size() > 1) {
				std::sort(layers[sortable_layer].begin(), layers[sortable_layer].end(), [](entity_id b, entity_id a) {
					return components::physics::are_connected_by_friction(a, b);
				});
			}
		}
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
		if (it.dead())
			continue;

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

			if (render.last_step_when_visible == current_step - 1) {
				components::transform interpolated_transform;
				interpolated_transform.pos = actual_transform.pos * ratio + render.previous_transform.pos * (1.0f - ratio);
				interpolated_transform.rotation = vec2().set_from_degrees(render.previous_transform.rotation).lerp(vec2().set_from_degrees(actual_transform.rotation), ratio).degrees();

				if ((actual_transform.pos - interpolated_transform.pos).length_sq() > 1.f)
					actual_transform.pos = interpolated_transform.pos;
				if (tabs(actual_transform.rotation - interpolated_transform.rotation) > 1.f)
					actual_transform.rotation = interpolated_transform.rotation;
			}
			else {
				render.previous_transform = actual_transform;
			}

			render.last_step_when_visible = current_step;
		}
	}

	++current_step;
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

void render_system::draw_layer(state_for_drawing_camera in_camera, int layer) {
	state_for_drawing_renderable in;
	in.setup_camera_state(in_camera);

	if (layer < layers.size() && !layers[layer].empty()) {
		for (auto e : layers[layer]) {
			auto& render = e->get<components::render>();

			in.renderable_transform = e->get<components::transform>();
			in.renderable_transform.pos = vec2i(in.renderable_transform.pos);
			in.renderable_transform.rotation = in.renderable_transform.rotation;

			in.renderable = e;

			in.camera_transform = render.absolute_transform ? components::transform() : in_camera.camera_transform;

			auto* polygon = e->find<components::polygon>();
			auto* sprite = e->find<components::sprite>();
			auto* tile_layer = e->find<components::tile_layer>();

			if (polygon) polygon->draw(in);
			if (sprite) sprite->draw(in);
			if (tile_layer) tile_layer->draw(in);
		}
	}
}

void render_system::draw_all_visible_entities(state_for_drawing_camera in, int mask) {
	generate_layers_from_visible_entities(mask);

	for (size_t i = 0; i < layers.size(); ++i)
		draw_layer(in, layers.size()-i-1);
}

