#pragma once
#include <vector>
#include <optional>
#include <Box2D/Collision/b2DynamicTree.h>

#include "augs/misc/enum/enum_array.h"

#include "game/enums/tree_of_npo_type.h"
#include "game/inferred_caches/inferred_cache_common.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/math/camera_cone.h"
/* NPO stands for "non-physical objects" */

namespace components {
	struct tree_of_npo_node;
};

union tree_of_npo_node {
	using payload_type = unversioned_entity_id;
	static_assert(sizeof(payload_type) <= sizeof(void*));

	payload_type payload;
	void* bytes;

	tree_of_npo_node() {
		bytes = nullptr;
	}
};

struct tree_of_npo_node_input {		
	tree_of_npo_type type = tree_of_npo_type::RENDERABLES;
	ltrb aabb;
};

class tree_of_npo_cache {
	struct tree {
		b2DynamicTree nodes;
	};

	augs::enum_array<tree, tree_of_npo_type> trees;

	struct cache {
		tree_of_npo_type type = tree_of_npo_type::COUNT;
		ltrb recorded_aabb;
		int tree_proxy_id = -1;

		void clear(tree_of_npo_cache& owner);
	};

	inferred_cache_map<cache> per_entity_cache;

	tree& get_tree(const cache&);

	cache* find_cache(const unversioned_entity_id);
	const cache* find_cache(const unversioned_entity_id) const;

public:
	/* Used for example for debugging the created nodes */
	template <class F>
	void for_each_aabb(F callback) const {
		for (const auto& cache : per_entity_cache) {
			callback(cache.second.recorded_aabb);
		}
	}

	template <class F>
	void for_each_in_camera(
		F callback,
		const camera_cone cone,
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
		const auto visible_aabb = cone.get_visible_world_rect_aabb();

		b2AABB input;
		input.lowerBound = b2Vec2(visible_aabb.left_top());
		input.upperBound = b2Vec2(visible_aabb.right_bottom());

		tree.nodes.Query(&aabb_listener, input);
	}

	void reserve_caches_for_entities(const size_t n);

	void infer_cache_for(const const_entity_handle);
	void destroy_cache_of(const const_entity_handle);
};