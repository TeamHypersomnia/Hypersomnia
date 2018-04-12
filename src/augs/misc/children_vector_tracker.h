#pragma once
#include <array>
#include <unordered_map>

#include "augs/templates/container_templates.h"
#include "augs/ensure.h"

namespace augs {
	/* 
		For joints, instead of having two children vectors for a single parent cache,
		we'll just have two trackers named:
		- joints_of_bodies_A, and
		- joints_of_bodies_B.

		Functionality will be symmetrical and this code can thus be made a lot easier.
	*/

	template <class child_id_t, class parent_id_t>
	class children_vector_tracker {
	public:
		using parent_id_type = parent_id_t;
		using child_id_type = child_id_t;

		using children_vector_type = std::vector<child_id_type>;
	private:
		struct parent_state {
			children_vector_type tracked_children;

			bool empty() const {
				return tracked_children.empty();
			}
		};

		std::unordered_map<parent_id_type, parent_state> parent_caches;

	public:
		void unset_parenthood(
			const child_id_type child_id,
			const parent_id_type parent_id
		) {
			if (auto parent_cache = mapped_or_nullptr(parent_caches, parent_id)) {
				auto& tracked_children = parent_cache->tracked_children;

				const auto previous_size = tracked_children.size();
				erase_element(tracked_children, child_id);

				/* Ensure that erasure happened */
				ensure_eq(tracked_children.size(), previous_size - 1); 

				if (parent_cache->empty()) {
					erase_element(parent_caches, parent_id);
				}
			}
			else {
				ensure(false && "Trying to unset a non-existing parent.")
			}
		}

		void assign_parenthood(
			const child_id_type child_id, 
			const parent_id_type new_parent_id
		) {
			auto& parent_cache = parent_caches[new_parent_id];
			auto& tracked_children = parent_cache.tracked_children;

			ensure(!found_in(tracked_children, child_id));
			tracked_children.push_back(child_id);
		}
		
		void reassign_parenthood(
			const child_id_type child_id, 
			const parent_id_type previous_parent_id,
			const parent_id_type parent_id
		) {
			unset_parenthood(child_id, previous_parent_id);
			assign_parenthood(child_id, parent_id);
		}

		const auto& get_children_of(const parent_id_type parent) const {
			thread_local children_vector_type zero;

			if (const auto p = mapped_or_nullptr(parent_caches, parent)) {
				return p->tracked_children;
			}

			return zero;
		}
	};
}