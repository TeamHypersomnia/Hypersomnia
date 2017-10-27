#pragma once
#include <array>
#include <vector>

#include "augs/templates/container_templates.h"
#include "augs/ensure.h"

namespace augs {
	template <class id_type, std::size_t parent_count>
	class children_vector_tracker {
	public:
		using parent_array_type = std::array<id_type, parent_count>;
	private:
		struct parent_state {
			std::array<
				std::vector<id_type>,
				parent_count
			> children;
		};

		struct child_state {
			parent_array_type parents;
		};

		std::vector<child_state> child_caches;
		std::vector<parent_state> parent_caches;

	public:
		void reserve(const std::size_t n) {
			child_caches.resize(n);
			parent_caches.resize(n);
		}
		
		//bool has_any_children(const id_type id) const {
		//	return parents[linear_cache_key(id)].children.size() > 0;
		//}

		void handle_deletion_of(
			const id_type this_id
		) {
			const auto& cache = parent_caches[linear_cache_key(this_id)];

			/*
				If it is a parent, clear all references to it
			*/

			for(const auto& children_vector : cache.children) {
				for (const auto& c : children_vector) {
					for (std::size_t p = 0; p < parent_count; ++p) {
						auto& parent_reference = child_caches[linear_cache_key(c)].parents.at(p);

						if (parent_reference == this_id) {
							parent_reference = id_type();
						}
					}
				}
			}
		}

		bool is_parent_set(
			const id_type child_id,
			const std::size_t parent_index
		) {
			auto& child_cache = child_caches[linear_cache_key(child_id)];
			auto& parent_entry = child_cache.parents.at(parent_index);

			return parent_entry.is_set();
		}

		template <class = std::enable_if_t<parent_count == 1>>
		bool is_parent_set(
			const id_type child_id
		) {
			return is_parent_set(child_id, 0u);
		}

		void unset_parent_of(
			const id_type child_id,
			const std::size_t parent_index
		) {
			auto& child_cache = child_caches[linear_cache_key(child_id)];
			auto& parent_entry = child_cache.parents.at(parent_index);

			if (parent_entry.is_set()) {
				auto& parent_cache = parent_caches[linear_cache_key(parent_entry)];

				parent_entry = id_type();
				erase_element(parent_cache.children.at(parent_index), child_id);
			}
		}

		template <class = std::enable_if_t<parent_count == 1>>
		void unset_parent_of(
			const id_type child_id
		) {
			unset_parent_of(child_id, 0u);
		}

		void unset_parents_of(const id_type child_id) {
			for (std::size_t p = 0; p < parent_count; ++p) {
				unset_parent_of(child_id, p);
			}
		}

		void set_parent(
			const id_type child_id, 
			const id_type parent_id,
			const std::size_t parent_index
		) {
			unset_parent_of(child_id, parent_index);

			auto& child_cache = child_caches[linear_cache_key(child_id)];
			child_cache.parents.at(parent_index) = parent_id;
			
			for (std::size_t p = 0; p < parent_count; ++p) {
				if (p != parent_index) {
					const bool the_same_parent_in_different_slot_exists =
						child_cache.parents.at(p) == parent_id
					;

					ensure(!the_same_parent_in_different_slot_exists);
				}
			}

			auto& parent_cache = parent_caches[linear_cache_key(parent_id)];
			parent_cache.children.at(parent_index).push_back(child_id);
		}

		template <class = std::enable_if_t<parent_count == 1>>
		void set_parent(
			const id_type child_id, 
			const id_type parent_id
		) {
			set_parent(child_id, parent_id, 0u);
		}

		template <class = std::enable_if_t<parent_count == 1>>
		const auto& get_children_of(
			const id_type parent
		) const {
			return parent_caches[linear_cache_key(parent)].children.at(0);
		}

		const auto& get_children_of(
			const id_type parent, 
			const std::size_t parenthood_index
		) const {
			return parent_caches[linear_cache_key(parent)].children.at(parenthood_index);
		}

		auto get_all_children_of(
			const id_type parent 
		) const {
			std::vector<id_type> all_children;

			for (std::size_t p = 0; p < parent_count; ++p) {
				concatenate(all_children, get_children_of(parent, p));
			}

			return all_children;
		}

		void set_parents(
			const id_type child_id, 
			const parent_array_type parents
		) {
			for (std::size_t p = 0; p < parent_count; ++p) {
				set_parent(child_id, parents.at(p), p);
			}
		}
	};
}