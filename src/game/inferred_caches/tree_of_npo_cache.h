#pragma once
#include <vector>
#include <Box2D/Collision/b2DynamicTree.h>

#include "augs/misc/enum/enum_array.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/components/tree_of_npo_node_component.h"
#include "augs/math/camera_cone.h"

class physics_world_cache;
struct cosmos_common_state;

/* NPO stands for "non-physical objects" */

union tree_of_npo_node {
	using payload_type = unversioned_entity_id;
	static_assert(sizeof(payload_type) <= sizeof(void*));

	payload_type payload;
	void* bytes;

	tree_of_npo_node() {
		bytes = nullptr;
	}
};

class tree_of_npo_cache {
	friend class cosmos;
	
	friend class component_synchronizer<false, components::tree_of_npo_node>;

	struct tree {
		b2DynamicTree nodes;
	};

	augs::enum_array<tree, tree_of_npo_type> trees;

	struct cache {
		bool constructed = false;
		tree_of_npo_type type = tree_of_npo_type::COUNT;
		int tree_proxy_id = -1;

		bool is_constructed() const;
	};

	std::vector<cache> per_entity_cache;

	void reserve_caches_for_entities(const size_t n);
	void infer_cache_for(const const_entity_handle);
	void destroy_cache_of(const const_entity_handle);

	tree& get_tree(const cache&);
	cache& get_cache(const unversioned_entity_id);

public:
	template <class F>
	void for_each_in_camera(
		F callback,
		const camera_cone camera,
		const vec2 screen_size,
		const tree_of_npo_type type
	) const {
		const auto& tree = trees[type];

		struct render_listener {
			const b2DynamicTree* const tree;
			F callback;

			bool QueryCallback(const int32 node_id) const {
				tree_of_npo_node node;
				node.bytes = tree->GetUserData(node_id);
				
				callback(node.payload);
				return true;
			}
		};

		const auto aabb_listener = render_listener{ &tree.nodes, callback };
		const auto visible_aabb = camera.get_visible_world_rect_aabb(screen_size);

		b2AABB input;
		input.lowerBound = b2Vec2(visible_aabb.left_top());
		input.upperBound = b2Vec2(visible_aabb.right_bottom());

		tree.nodes.Query(&aabb_listener, input);
	}
};