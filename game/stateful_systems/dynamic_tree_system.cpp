#include "dynamic_tree_system.h"
#include "game/step.h"
#include "game/cosmos.h"
#include "game/entity_handle.h"
#include "game/components/dynamic_tree_node_component.h"
#include "game/components/camera_component.h"
#include "game/enums/filters.h"

void dynamic_tree_system::destruct(const_entity_handle handle) {
	auto id = handle.get_id();
	size_t index = id.indirection_index;

	auto& cache = per_entity_cache[index];

	if (cache.is_constructed) {
		remove_element(always_visible_entities, entity_id(handle));

		if (cache.tree_proxy_id != -1) {
			auto* userdata = non_physical_objects_tree.GetUserData(cache.tree_proxy_id);
			ensure(userdata != nullptr);
			delete ((entity_id*)userdata);
			non_physical_objects_tree.DestroyProxy(cache.tree_proxy_id);
		}

		per_entity_cache[index].is_constructed = false;
	}
}

void dynamic_tree_system::construct(const_entity_handle handle) {
	if (!handle.has<components::dynamic_tree_node>()) return;

	auto id = handle.get_id();
	size_t index = id.indirection_index;

	auto& cache = per_entity_cache[index];

	ensure(!cache.is_constructed);

	auto& dynamic_tree_node = handle.get<components::dynamic_tree_node>();

	if (dynamic_tree_node.is_activated()) {
		auto& data = dynamic_tree_node.get_data();

		if (data.always_visible) {
			always_visible_entities.push_back(handle);
		}
		else {
			b2AABB input;
			input.lowerBound = data.aabb.left_top();
			input.upperBound = data.aabb.right_bottom();

			cache.tree_proxy_id = non_physical_objects_tree.CreateProxy(input, new entity_id(handle));
		}

		cache.is_constructed = true;
	}
}

void dynamic_tree_system::reserve_caches_for_entities(size_t n) {
	per_entity_cache.resize(n);
}

std::vector<entity_id> dynamic_tree_system::determine_visible_entities_from_camera(const_entity_handle camera) const {
	auto& cosmos = camera.get_cosmos();
	std::vector<entity_id> visible_entities;

	auto& physics = cosmos.stateful_systems.get<physics_system>();
	auto& in = camera.get<components::camera>().how_camera_will_render;

	auto& result = physics.query_aabb_px(in.transformed_visible_world_area_aabb.left_top(), in.transformed_visible_world_area_aabb.right_bottom(), filters::renderable_query());
	visible_entities.insert(visible_entities.end(), result.entities.begin(), result.entities.end());

	struct render_listener {
		const b2DynamicTree* tree;
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

	std::sort(visible_entities.begin(), visible_entities.end());
	visible_entities.erase(std::unique(visible_entities.begin(), visible_entities.end()), visible_entities.end());

	return visible_entities;
}
