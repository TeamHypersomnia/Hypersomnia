#include "dynamic_tree_system.h"
#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/components/dynamic_tree_node_component.h"
#include "game/enums/filters.h"
#include "augs/templates/container_templates.h"

bool dynamic_tree_system::cache::is_constructed() const {
	return constructed;
}

dynamic_tree_system::cache& dynamic_tree_system::get_cache(const unversioned_entity_id id) {
	return per_entity_cache[id.pool.indirection_index];
}

dynamic_tree_system::tree& dynamic_tree_system::get_tree(const cache& c) {
	return trees[static_cast<size_t>(c.type)];
}

void dynamic_tree_system::destruct(const const_entity_handle handle) {
	auto& cache = get_cache(handle.get_id());

	if (cache.is_constructed()) {
		remove_element(get_tree(cache).always_visible, handle.get_id());

		if (cache.tree_proxy_id != -1) {
			get_tree(cache).nodes.DestroyProxy(cache.tree_proxy_id);
		}

		cache = dynamic_tree_system::cache();
	}
}

void dynamic_tree_system::construct(const const_entity_handle handle) {
	if (!handle.has<components::dynamic_tree_node>()) {
		return;
	}

	auto& cache = get_cache(handle.get_id());

	ensure(!cache.is_constructed());

	const auto& dynamic_tree_node = handle.get<components::dynamic_tree_node>();

	if (dynamic_tree_node.is_activated()) {
		auto& data = dynamic_tree_node.get_data();

		cache.type = data.type;

		if (data.always_visible) {
			get_tree(cache).always_visible.push_back(handle.get_id());
		}
		else {
			b2AABB input;
			input.lowerBound = data.aabb.left_top();
			input.upperBound = data.aabb.right_bottom();
			
			unversioned_entity_id node_userdata(handle.get_id());
			static_assert(sizeof(node_userdata) <= sizeof(void*), "Userdata must be less than size of void*");

			cache.tree_proxy_id = get_tree(cache).nodes.CreateProxy(input, reinterpret_cast<void*>(node_userdata.pool.indirection_index));
		}
		
		cache.constructed = true;
	}
}

void dynamic_tree_system::reserve_caches_for_entities(const size_t n) {
	per_entity_cache.resize(n);
}

std::vector<unversioned_entity_id> dynamic_tree_system::determine_visible_entities_from_camera(
	const camera_cone in,
	const components::dynamic_tree_node::tree_type type
) const {
	const auto& tree = trees[static_cast<size_t>(type)];

	std::vector<unversioned_entity_id> visible_entities = tree.always_visible;

	const auto visible_aabb = in.get_transformed_visible_world_area_aabb();

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

	aabb_listener.tree = &tree.nodes;
	aabb_listener.visible_entities = &visible_entities;

	b2AABB input;
	input.lowerBound = visible_aabb.left_top();
	input.upperBound = visible_aabb.right_bottom();

	tree.nodes.Query(&aabb_listener, input);

	return visible_entities;
}
