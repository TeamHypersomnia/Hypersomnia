#include "tree_of_npo_system.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/components/tree_of_npo_node_component.h"
#include "game/enums/filters.h"
#include "augs/templates/container_templates.h"

bool tree_of_npo_system::cache::is_constructed() const {
	return constructed;
}

tree_of_npo_system::cache& tree_of_npo_system::get_cache(const unversioned_entity_id id) {
	return per_entity_cache[make_cache_id(id)];
}

tree_of_npo_system::tree& tree_of_npo_system::get_tree(const cache& c) {
	return trees[static_cast<size_t>(c.type)];
}

void tree_of_npo_system::destroy_inferred_state_of(const const_entity_handle handle) {
	auto& cache = get_cache(handle.get_id());

	if (cache.is_constructed()) {
		remove_element(get_tree(cache).always_visible, handle.get_id());

		if (cache.tree_proxy_id != -1) {
			get_tree(cache).nodes.DestroyProxy(cache.tree_proxy_id);
		}

		cache = tree_of_npo_system::cache();
	}
}

void tree_of_npo_system::create_inferred_state_for(const const_entity_handle handle) {
	if (!handle.has<components::tree_of_npo_node>()) {
		return;
	}

	auto& cache = get_cache(handle.get_id());

	ensure(!cache.is_constructed());

	const auto& tree_of_npo_node = handle.get<components::tree_of_npo_node>();

	if (tree_of_npo_node.is_activated()) {
		const auto data = tree_of_npo_node.get_raw_component();

		cache.type = data.type;

		if (data.always_visible) {
			get_tree(cache).always_visible.push_back(handle.get_id());
		}
		else {
			b2AABB input;
			input.lowerBound = data.aabb.left_top();
			input.upperBound = data.aabb.right_bottom();
			
			auto node_userdata = handle.get_id().operator unversioned_entity_id();
			static_assert(sizeof(node_userdata) <= sizeof(void*), "Userdata must be less than size of void*");

			cache.tree_proxy_id = get_tree(cache).nodes.CreateProxy(input, reinterpret_cast<void*>(node_userdata.indirection_index));
		}
		
		cache.constructed = true;
	}
}

void tree_of_npo_system::reserve_caches_for_entities(const size_t n) {
	per_entity_cache.resize(n);
}

void tree_of_npo_system::determine_visible_entities_from_camera(
	std::vector<unversioned_entity_id>& into,
	const camera_cone in,
	const tree_of_npo_type type
) const {
	const auto& tree = trees[type];

	concatenate(into, tree.always_visible);

	struct render_listener {
		const b2DynamicTree* tree;
		std::vector<unversioned_entity_id>* visible_entities;
		bool QueryCallback(int32 node) {
			unversioned_entity_id id;
			id.indirection_index = reinterpret_cast<int>(tree->GetUserData(node));
			static_assert(std::is_same<decltype(id.indirection_index), int>::value, "Userdata types incompatible");

			visible_entities->push_back(id);
			return true;
		}
	};

	render_listener aabb_listener;

	aabb_listener.tree = &tree.nodes;
	aabb_listener.visible_entities = &into;
	
	const auto visible_aabb = in.get_transformed_visible_world_area_aabb().expand_from_center({ 50, 50 });

	b2AABB input;
	input.lowerBound = visible_aabb.left_top();
	input.upperBound = visible_aabb.right_bottom();

	tree.nodes.Query(&aabb_listener, input);
}
