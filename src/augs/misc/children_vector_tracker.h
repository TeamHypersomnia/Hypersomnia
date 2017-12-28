#pragma once
#include <array>
#include <unordered_map>

#include "augs/templates/container_templates.h"
#include "augs/ensure.h"

namespace augs {
	template <
		class child_id_t,
		class parent_id_t, 
		std::size_t parent_count
	>
	class children_vector_tracker {
	public:
		using parent_id_type = parent_id_t;
		using child_id_type = child_id_t;

		using parent_array_type = std::array<parent_id_type, parent_count>;
		using children_vector_type = std::vector<child_id_type>;
	private:
		struct parent_state {
			std::array<children_vector_type, parent_count> children_vectors;

			bool all_empty() const {
				for (const auto& c : children_vectors) {
					if (c.size() > 0) {
						return false;
					}
				}

				return true;
			}
		};

		struct child_state {
			parent_array_type parents;

			bool orphan() const {
				for (const auto& p : parents) {
					if (p.is_set()) {
						return false;
					}
				}

				return true;
			}
		};

		std::unordered_map<child_id_type, child_state> child_caches;
		std::unordered_map<parent_id_type, parent_state> parent_caches;

	public:
		void reserve(const std::size_t n) {

		}
		
		void handle_deletion_of_parent(
			const parent_id_type this_id
		) {
			if (const auto cache = mapped_or_nullptr(parent_caches, this_id)) {
				/* Clear all references to it in the caches of the children of this parent */

				for (const auto& children_vector : cache->children_vectors) {
					for (const auto& c : children_vector) {
						for (std::size_t p = 0; p < parent_count; ++p) {
							auto& child_cache = child_caches.at(c);
							auto& parent_id = child_cache.parents.at(p);

							if (parent_id == this_id) {
								parent_id = parent_id_type();
							}

							if (child_cache.orphan()) {
								erase_element(child_caches, c);
								break;
							}
						}
					}
				}

				erase_element(parent_caches, this_id);
			}
		}

		bool is_parent_set(
			const child_id_type child_id,
			const std::size_t parent_index
		) const {
			if (const auto child_cache = mapped_or_nullptr(child_caches, child_id)) {
				auto& parent_entry = child_cache->parents.at(parent_index);
				return parent_entry.is_set();
			}

			return false;
		}

		template <bool C = parent_count == 1, class = std::enable_if_t<C>>
		bool is_parent_set(
			const child_id_type child_id
		) {
			return is_parent_set(child_id, 0u);
		}

		void unset_parent_of(
			const child_id_type child_id,
			const std::size_t parent_index
		) {
			if (auto child_cache = mapped_or_nullptr(child_caches, child_id)) {
				auto& parent_id = child_cache->parents.at(parent_index);

				if (parent_id.is_set()) {
					if (auto parent_cache = mapped_or_nullptr(parent_caches, parent_id)) {
						erase_element(parent_cache->children_vectors.at(parent_index), child_id);

						if (parent_cache->all_empty()) {
							erase_element(parent_caches, parent_id);
						}
					}

					parent_id = parent_id_type();
				}

				if (child_cache->orphan()) {
					erase_element(child_caches, child_id);
				}
			}
		}

		template <bool C = parent_count == 1, class = std::enable_if_t<C>>
		void unset_parent_of(
			const child_id_type child_id
		) {
			unset_parent_of(child_id, 0u);
		}

		void unset_parents_of(const child_id_type child_id) {
			for (std::size_t p = 0; p < parent_count; ++p) {
				unset_parent_of(child_id, p);
			}
		}

		void set_parent(
			const child_id_type child_id, 
			const parent_id_type parent_id,
			const std::size_t parent_index
		) {
			unset_parent_of(child_id, parent_index);

			if (!parent_id.is_set()) {
				return;
			}

			auto& child_cache = child_caches[child_id];
			child_cache.parents.at(parent_index) = parent_id;
			
			for (std::size_t p = 0; p < parent_count; ++p) {
				if (p != parent_index) {
					const bool the_same_parent_in_different_slot_exists =
						child_cache.parents.at(p) == parent_id
					;

					ensure(!the_same_parent_in_different_slot_exists);
				}
			}

			auto& parent_cache = parent_caches[parent_id];
			parent_cache.children_vectors.at(parent_index).push_back(child_id);
		}

		template <bool C = parent_count == 1, class = std::enable_if_t<C>>
		void set_parent(
			const child_id_type child_id, 
			const parent_id_type parent_id
		) {
			set_parent(child_id, parent_id, 0u);
		}


		const auto& get_children_of(
			const parent_id_type parent, 
			const std::size_t parenthood_index
		) const {
			thread_local children_vector_type zero;

			if (const auto p = mapped_or_nullptr(parent_caches, parent)) {
				return p->children_vectors.at(parenthood_index);
			}

			return zero;
		}

		template <bool C = parent_count == 1, class = std::enable_if_t<C>>
		const auto& get_children_of(
			const parent_id_type parent
		) const {
			return get_children_of(parent, 0u);
		}

		template <bool C = (parent_count>1), class = std::enable_if_t<C>>
		auto get_all_children_of(
			const parent_id_type parent 
		) const {
			thread_local std::vector<child_id_type> all_children;
			all_children.clear();

			for (std::size_t p = 0; p < parent_count; ++p) {
				concatenate(all_children, get_children_of(parent, p));
			}

			return all_children;
		}

		void set_parents(
			const child_id_type child_id, 
			const parent_array_type parents
		) {
			for (std::size_t p = 0; p < parent_count; ++p) {
				set_parent(child_id, parents.at(p), p);
			}
		}
	};
}