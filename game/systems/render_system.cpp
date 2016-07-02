#include <algorithm>

#include "render_system.h"
#include "game/entity_id.h"
#include "game/detail/state_for_drawing_camera.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/tile_layer_component.h"
#include "game/components/physics_component.h"
#include "game/components/particle_group_component.h"

#include "game/messages/new_entity_message.h"
#include "game/messages/queue_destruction.h"

#include "physics_system.h"
#include "camera_system.h"

#include "game/cosmos.h"
#include "game/enums/filters.h"

#include <algorithm>

using namespace shared;

render_system::render_system(cosmos& parent_cosmos) : processing_system_with_cosmos_reference(parent_cosmos) {
	layers_whose_order_determines_friction.push_back(render_layer::CAR_INTERIOR);
}

void render_system::remove_entities_from_rendering_tree() {
	auto& events = step.messages.get_queue<messages::queue_destruction>();

	for (auto& it : events) {
		auto& e = it.subject;

		auto* physics_definition = e.find<components::physics_definition>();
		auto* render = e.find<components::render>();

		if (render && render->tree_proxy_id >= 0) {
			auto* userdata = non_physical_objects_tree.GetUserData(render->tree_proxy_id);
			
			if (userdata) {
				delete ((entity_id*)userdata);
				non_physical_objects_tree.DestroyProxy(render->tree_proxy_id);
			}
		}
	}
}

void render_system::determine_visible_entities_from_every_camera() {
	auto& cameras = parent_cosmos.stateful_systems.get<camera_system>().targets;
	auto& physics = parent_cosmos.stateful_systems.get<physics_system>();

	visible_entities.clear();

	for (auto& camera : cameras) {
		auto& in = camera.get<components::camera>().how_camera_will_render;

		auto& result = physics.query_aabb_px(in.transformed_visible_world_area_aabb.left_top(), in.transformed_visible_world_area_aabb.right_bottom(), filters::renderable_query());
		visible_entities.insert(visible_entities.end(), result.entities.begin(), result.entities.end());

		visible_entities.erase(
			std::remove_if(visible_entities.begin(), visible_entities.end(), 
				[](entity_id id) { 
			return id.find<components::render>() == nullptr;  
		}), visible_entities.end());

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

	std::sort(visible_entities.begin(), visible_entities.end());
	visible_entities.erase(std::unique(visible_entities.begin(), visible_entities.end()), visible_entities.end());
}

void render_system::generate_layers_from_visible_entities(int mask) {
	layers.clear();

	/* shortcut */
	std::vector<entity_id> entities_by_mask;

	for (auto& it : visible_entities) {
		auto* maybe_render = it.find<components::render>();
		if (!maybe_render) continue;

		if (maybe_render->mask == mask)
			entities_by_mask.push_back(it);
	}

	for (auto& it : entities_by_mask) {
		auto layer = it.get<components::render>().layer;
		
		if (layer >= layers.size()) 
			layers.resize(layer+1);

		layers[layer].push_back(it);
	}

	for (auto& custom_order_layer : layers_whose_order_determines_friction) {
		if (custom_order_layer < layers.size()) {
			if (layers[custom_order_layer].size() > 1) {
				std::sort(layers[custom_order_layer].begin(), layers[custom_order_layer].end(), [](entity_id b, entity_id a) {
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


void render_system::standard_draw_entity(entity_id e, shared::state_for_drawing_camera in_camera, bool only_border_highlights, int visibility_index) {
	static thread_local state_for_drawing_renderable in;
	in.setup_camera_state(in_camera);

	auto& render = e.get<components::render>();

	in.renderable_transform = e.get<components::transform>();
	in.renderable_transform.pos = vec2i(in.renderable_transform.pos);
	in.renderable_transform.rotation = in.renderable_transform.rotation;

	in.renderable = e;

	in.camera_transform = render.absolute_transform ? components::transform() : in_camera.camera_transform;

	auto* polygon = e.find<components::polygon>();
	auto* sprite = e.find<components::sprite>();
	auto* tile_layer = e.find<components::tile_layer>();
	auto* particle_group = e.find<components::particle_group>();

	if (only_border_highlights) {
		if (render.draw_border) {
			static vec2i offsets[4] = {
				vec2i(-1, 0), vec2i(1, 0), vec2i(0, 1), vec2i(0, -1)
				//vec2i(-1, -1), vec2i(1, 1), vec2i(-1, 1), vec2i(1, -1)
			};

			auto original_pos = in.renderable_transform.pos;

			in.colorize = render.border_color;

			for (auto& o : offsets) {
				in.renderable_transform.pos = original_pos + o;

				if (polygon) polygon->draw(in);
				if (sprite) sprite->draw(in);
				if (particle_group) particle_group->draw(in);
			}

			in.renderable_transform.pos = original_pos;
			in.colorize = white;
		}
	}
	else {
		if (polygon) polygon->draw(in);
		if (sprite) sprite->draw(in);
		if (tile_layer) tile_layer->draw(in);
		if (particle_group) particle_group->draw(in);
		
		if(visibility_index > -1)
			render.last_visibility_index = visibility_index;
	}
}

void render_system::draw_layer(state_for_drawing_camera in_camera, int layer, bool only_border_highlights) {
	state_for_drawing_renderable in;
	in.setup_camera_state(in_camera);

	if (layer < layers.size() && !layers[layer].empty()) {
		for (auto e : layers[layer]) {
			standard_draw_entity(e, in_camera, only_border_highlights, current_visibility_index);

			if (!only_border_highlights)
				++current_visibility_index;
		}
	}
}

void render_system::draw_all_visible_entities(state_for_drawing_camera in, int mask) {
	generate_layers_from_visible_entities(mask);

	for (size_t i = 0; i < layers.size(); ++i)
		draw_layer(in, layers.size()-i-1);
}

const std::vector<entity_id>& render_system::get_all_visible_entities() const {
	return visible_entities;
}

