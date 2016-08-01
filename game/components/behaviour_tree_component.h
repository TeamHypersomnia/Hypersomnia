#pragma once
#include <vector>

#include "game/resources/behaviour_tree.h"
#include "game/assets/behaviour_tree_id.h"

#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"

namespace components {
	struct behaviour_tree {
		struct instance {
			resources::behaviour_tree::state_of_tree_instance state;
			assets::behaviour_tree_id tree_id;

			template <class Archive>
			void serialize(Archive& ar) {
				ar(CEREAL_NVP(state), 
					CEREAL_NVP(tree_id));
			}
		};

		augs::constant_size_vector<instance, CONCURRENT_TREES_COUNT> concurrent_trees;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(CEREAL_NVP(concurrent_trees));
		}
	};
}

