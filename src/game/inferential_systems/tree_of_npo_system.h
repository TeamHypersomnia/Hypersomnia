#pragma once
#include <vector>
#include <Box2D/Collision/b2DynamicTree.h>

#include "augs/misc/enum_array.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/components/tree_of_npo_node_component.h"
#include "augs/math/camera_cone.h"

class physics_system;
struct cosmos_common_state;

/* NPO stands for "non-physical objects" */

class tree_of_npo_system {
	friend class cosmos;
	
	friend class component_synchronizer<false, components::tree_of_npo_node>;

	struct tree {
		std::vector<unversioned_entity_id> always_visible;
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
	void create_inferred_state_for(const const_entity_handle);
	void destroy_inferred_state_of(const const_entity_handle);

	void create_additional_inferred_state(const cosmos_common_state&) {}
	void destroy_additional_inferred_state(const cosmos_common_state&) {}

	tree& get_tree(const cache&);
	cache& get_cache(const unversioned_entity_id);

public:
	template <class F>
	void for_each_visible_in_camera(
		F callback,
		const camera_cone camera,
		const tree_of_npo_type type
	) const {
		const auto& tree = trees[type];

		for (const auto e : tree.always_visible) {
			callback(e);
		}

		struct render_listener {
			const b2DynamicTree* tree;
			F callback;

			bool QueryCallback(const int32 node) const {
				unversioned_entity_id id;
				id.indirection_index = reinterpret_cast<decltype(id.indirection_index)>(tree->GetUserData(node));

				callback(id);
				return true;
			}
		};

		const auto aabb_listener = render_listener{ &tree.nodes, callback };
		const auto visible_aabb = camera.get_transformed_visible_world_area_aabb().expand_from_center({ 50, 50 });

		b2AABB input;
		input.lowerBound = visible_aabb.left_top();
		input.upperBound = visible_aabb.right_bottom();

		tree.nodes.Query(&aabb_listener, input);
	}
};