#pragma once
#include <cstddef>
#include <vector>
#include <optional>
#include <Box2D/Collision/b2DynamicTree.h>

#include "augs/misc/enum/enum_array.h"

#include "game/enums/tree_of_npo_type.h"
#include "game/cosmos/entity_handle_declaration.h"

#include "augs/math/camera_cone.h"

#include "game/detail/entities_with_render_layer.h"
#include "game/cosmos/entity_type_traits.h"
#include "game/cosmos/entity_id.h"
#include "game/inferred_caches/tree_of_npo_cache_data.h"

/* NPO stands for "non-physical objects" */

namespace components {
	struct tree_of_npo_node;
};

class cosmos;

struct tree_of_npo_node_input {		
	tree_of_npo_type type = tree_of_npo_type::RENDERABLES;
	ltrb aabb;
};

class tree_of_npo_cache {
	using cache = tree_of_npo_cache_data;
	friend cache;

	struct tree {
		b2DynamicTree nodes;
	};

	augs::enum_array<tree, tree_of_npo_type> trees;

	tree& get_tree(const cache&);

public:
	template <class E>
	struct concerned_with {
		static constexpr bool value = 
			is_one_of_list_v<E, entities_with_render_layer>
			&& !has_any_of_v<E, invariants::fixtures, invariants::rigid_body>
		;
	};	

	template <class F>
	void for_all_of_type(
		F callback,
		const tree_of_npo_type type
	) const {
		const auto& tree = trees[type];

		struct render_listener {
			const b2DynamicTree* const tree;
			F callback;

			bool QueryCallback(const int32 node_id) const {
				callback(tree->GetNode(node_id).payload1);
				return true;
			}
		};

		const auto aabb_listener = render_listener{ &tree.nodes, callback };

		tree.nodes.QueryAll(&aabb_listener);
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
				callback(tree->GetNode(node_id).payload1);
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

	void infer_all(cosmos&);

	template <class E>
	void specific_infer_cache_for(const E&);

	void infer_cache_for(const entity_handle&);
	void destroy_cache_of(const entity_handle&);
};