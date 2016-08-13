#include "dynamic_tree_system.h"
#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/components/dynamic_tree_node_component.h"
#include "game/enums/filters.h"

bool dynamic_tree_system::cache::is_constructed() const {
	return constructed;
}

void dynamic_tree_system::destruct(const_entity_handle handle) {
	auto id = handle.get_id();
	size_t index = id.pool.indirection_index;

	auto& cache = per_entity_cache[index];

	if (cache.is_constructed()) {
		remove_element(always_visible_entities, handle.get_id());

		if (cache.tree_proxy_id != -1)
			non_physical_objects_tree.DestroyProxy(cache.tree_proxy_id);

		per_entity_cache[index] = dynamic_tree_system::cache();
	}
}

void dynamic_tree_system::construct(const_entity_handle handle) {
	if (!handle.has<components::dynamic_tree_node>()) return;

	auto id = handle.get_id();
	size_t index = id.pool.indirection_index;

	auto& cache = per_entity_cache[index];

	ensure(!cache.is_constructed());

	auto& dynamic_tree_node = handle.get<components::dynamic_tree_node>();

	if (dynamic_tree_node.is_activated()) {
		auto& data = dynamic_tree_node.get_data();

		if (data.always_visible) {
			always_visible_entities.push_back(handle.get_id());
		}
		else {
			b2AABB input;
			input.lowerBound = data.aabb.left_top();
			input.upperBound = data.aabb.right_bottom();
			
			unversioned_entity_id node_userdata(handle.get_id());
			static_assert(sizeof(node_userdata) == sizeof(void*), "Userdata must be of size of void*");

			cache.tree_proxy_id = non_physical_objects_tree.CreateProxy(input, reinterpret_cast<void*>(node_userdata.pool.indirection_index));
		}
		
		cache.constructed = true;
	}
}

void dynamic_tree_system::reserve_caches_for_entities(size_t n) {
	per_entity_cache.resize(n);
}

std::vector<unversioned_entity_id> dynamic_tree_system::determine_visible_entities_from_camera(state_for_drawing_camera in, const physics_system& physics) const {
	std::vector<unversioned_entity_id> visible_entities = always_visible_entities;

	auto& result = physics.query_aabb_px(in.transformed_visible_world_area_aabb.left_top(), in.transformed_visible_world_area_aabb.right_bottom(), filters::renderable_query());
	visible_entities.insert(visible_entities.end(), result.entities.begin(), result.entities.end());

	struct render_listener {
		const b2DynamicTree* tree;
		std::vector<unversioned_entity_id>* visible_entities;
		bool QueryCallback(int32 node) {
			unversioned_entity_id id;
			id.pool.indirection_index = reinterpret_cast<int>(tree->GetUserData(node));
			static_assert(std::is_same<decltype(id.pool.indirection_index), int>::value, "Userdata types incompatible");

			visible_entities->push_back(id);
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
