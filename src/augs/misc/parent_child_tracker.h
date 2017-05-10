#include <array>
#include <vector>

#include "augs/ensure.h"
#include "augs/templates/container_templates.h"

namespace augs {
	template <class id_type, std::size_t parent_count>
	class parent_child_tracker {
		struct parent_state {
			std::array<
				std::vector<id_type>,
				parent_count
			> children;
		};

		struct child_state {
			std::array<id_type, parent_count> parents;
		};

		std::vector<child_state> child_caches;
		std::vector<parent_state> parent_caches;

	public:
		parent_child_tracker() {
			ensure(!is_valid_cache_id(id_type()));
		}

		void reserve(const std::size_t n) {
			child_caches.resize(n);
			parent_caches.resize(n);
		}
		
		//bool has_any_children(const id_type id) const {
		//	return parents[make_cache_id(id)].children.size() > 0;
		//}

		void handle_deletion_of(
			const id_type this_id
		) {
			const auto& cache = parent_caches[make_cache_id(this_id)];

			/*
				If it is a parent, clear all references to it
			*/

			for(const auto& children_vector : cache.children) {
				for (const auto& c : children_vector) {
					for (std::size_t p = 0; p < parent_count; ++p) {
						auto& parent_reference = child_caches[make_cache_id(c)].parents.at(p);

						if (parent_reference == this_id) {
							parent_reference = id_type();
						}
					}
				}
			}
		}

		bool is_parent_set(
			const id_type child_id,
			const std::size_t parent_index = 0u
		) {
			auto& child_cache = child_caches[make_cache_id(child_id)];
			auto& parent_entry = child_cache.parents.at(parent_index);

			return is_valid_cache_id(parent_entry);
		}

		void unset_parent_of(
			const id_type child_id,
			const std::size_t parent_index = 0u
		) {
			auto& child_cache = child_caches[make_cache_id(child_id)];
			auto& parent_entry = child_cache.parents.at(parent_index);

			const bool is_parent_set = is_valid_cache_id(parent_entry);

			if (is_parent_set) {
				auto& parent_cache = parent_caches[make_cache_id(parent_entry)];

				parent_entry = id_type();
				remove_element(parent_cache.children.at(parent_index), child_id);
			}
		}

		void unset_parents_of(const id_type child_id) {
			for (std::size_t p = 0; p < parent_count; ++p) {
				unset_parent_of(child_id, p);
			}
		}

		template <class H1, class H2>
		void set_parent(
			const H1 of_child_handle, 
			const H2 parent_handle,
			const std::size_t parent_index = 0u
		) {
			const auto parent_id = parent_handle.get_id();
			const auto child_id = of_child_handle.get_id();
			
			unset_parent_of(child_id, parent_index);

			ensure(of_child_handle.alive());
			ensure(parent_handle.alive());

			auto& child_cache = child_caches[make_cache_id(child_id)];
			child_cache.parents.at(parent_index) = parent_id;
			
			for (std::size_t p = 0; p < parent_count; ++p) {
				if (p != parent_index) {
					const bool the_same_parent_in_different_slot_exists =
						child_cache.parents.at(parent_index) == parent_id
					;

					ensure(!the_same_parent_in_different_slot_exists);
				}
			}

			auto& parent_cache = parent_caches[make_cache_id(parent_id)];
			parent_cache.children.at(parent_index).push_back(child_id);
		}

		template <class = std::enable_if_t<parent_count == 1>>
		auto& get_children_of(
			const id_type parent
		) const {
			return parent_caches[make_cache_id(parent)].children.at(0);
		}

		auto& get_children_of(
			const id_type parent, 
			const std::size_t parenthood_index
		) const {
			return parent_caches[make_cache_id(parent)].children.at(parenthood_index);
		}
	};
}