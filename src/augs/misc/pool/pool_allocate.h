#pragma once
#include "augs/misc/pool/pool.h"

namespace augs {
	template <class T, template <class> class M, class size_type, class... K>
	template <
		unsigned expansion_mult, 
		unsigned expansion_add, 
		class... Args
	>
	typename pool<T, M, size_type, K...>::allocation_result pool<T, M, size_type, K...>::allocate(Args&&... args) {
		if (full_capacity()) {
			if constexpr(constexpr_max_size) {
				throw std::runtime_error("This static pool cannot be further expanded.");
			}
			else {
				const auto old_size = size();
				const auto new_size = std::size_t(old_size) * expansion_mult + expansion_add;

				ensure(new_size <= std::numeric_limits<size_type>::max());

				reserve(static_cast<size_type>(new_size));
			}
		}

		const auto next_free_indirector = free_indirectors.back();
		free_indirectors.pop_back();
		
		const auto new_slot_index = size();

		pool_indirector_type& allocated_indirector = indirectors[next_free_indirector];
		allocated_indirector.real_index = new_slot_index;

		key_type allocated_id;
		allocated_id.version = allocated_indirector.version;
		allocated_id.indirection_index = next_free_indirector;

		pool_slot_type allocated_slot;
		allocated_slot.pointing_indirector = next_free_indirector;

		slots.push_back(allocated_slot);
		objects.emplace_back(std::forward<Args>(args)...);

		return { allocated_id, objects.back() };
	}

	template <class T, template <class> class M, class size_type, class... K>
	void pool<T, M, size_type, K...>::undo_last_allocate(const key_type key) {
		if (!correct_range(key)) {
			return;
		}

		auto& indirector = get_indirector(key);

		if (!versions_match(indirector, key)) {
			return;
		}

		if (const auto removed_at_index = indirector.real_index;
			removed_at_index != size() - 1
		) {
			throw std::runtime_error("Undoing allocations in bad order.");
		}
		else {
			/* add dead key's indirector to the list of free indirectors */
			free_indirectors.push_back(key.indirection_index);

			/* we do not increase version of the dead indirector, as we are undoing. */

			/* ...and mark it as unused. */
			indirector.real_index = static_cast<size_type>(-1);

			slots.pop_back();
			objects.pop_back();
		}
	}

	template <class T, template <class> class M, class size_type, class... K>
	auto pool<T, M, size_type, K...>::free(const key_type key) 
		-> std::optional<typename pool<T, M, size_type, K...>::undo_free_input_type>
	{
		if (!correct_range(key)) {
			return std::nullopt;
		}

		auto& indirector = get_indirector(key);

		if (!versions_match(indirector, key)) {
			return std::nullopt;
		}

		/*
			before we mess with the to-be-deleted indirector,
			save the stored real index 
		*/

		const auto removed_at_index = indirector.real_index;

		/* Prepare also the return value */
		undo_free_input_type result;
		result.indirection_index = key.indirection_index;
		result.real_index = removed_at_index;

		/* add dead key's indirector to the list of free indirectors */
		free_indirectors.push_back(key.indirection_index);

		/* therefore we must increase version of the dead indirector... */
		++indirector.version;

		/* ...and mark it as unused. */
		indirector.real_index = static_cast<size_type>(-1);

		if (removed_at_index != size() - 1) {
			{
				const auto indirector_of_last_element = slots.back().pointing_indirector;

				/* change last element's indirector - set it to the removed element's index */
				indirectors[indirector_of_last_element].real_index = removed_at_index;
			}

			slots[removed_at_index] = std::move(slots.back());
			objects[removed_at_index] = std::move(objects.back());
		}

		slots.pop_back();
		objects.pop_back();

		return result;
	}

	template <class T, template <class> class M, class size_type, class... K>
	auto pool<T, M, size_type, K...>::free(const unversioned_id_type key) {
		return free(to_versioned(key));
	}

	template <class T, template <class> class M, class size_type, class... K>
	template <class... Args>
	typename pool<T, M, size_type, K...>::allocation_result pool<T, M, size_type, K...>::undo_free(
		const typename pool<T, M, size_type, K...>::undo_free_input_type in,
		Args&&... removed_content
	) {
		static_assert(sizeof...(Args) > 0, "Specify what to construct.");

		const auto indirection_index = in.indirection_index;
		const auto real_index = in.real_index;

		if (free_indirectors.back() == indirection_index) {
			/* This will be usually the case */
			free_indirectors.pop_back();
		}
		else {
			/* This might happen if a reserve happened after free */
			erase_element(free_indirectors, indirection_index);
		}

		auto& indirector = indirectors[indirection_index];

		indirector.real_index = real_index;
		--indirector.version;

		auto get_new_key = [&](){
			key_type new_key;
			new_key.version = indirector.version;
			new_key.indirection_index = indirection_index;
			return new_key;
		};

		if (real_index < size()) {
			auto& new_slot_space = slots[real_index];
			auto& new_object_space = objects[real_index];

			{
				const auto indirector_of_moved_element = new_slot_space.pointing_indirector;

				/* change moved element's indirector - set it back to the last index */
				indirectors[indirector_of_moved_element].real_index = size();
			}

			slots.emplace_back(std::move(new_slot_space));
			objects.emplace_back(std::move(new_object_space));

			new_slot_space.pointing_indirector = indirection_index;
			new_object_space.~mapped_type();
			new (std::addressof(new_object_space)) mapped_type(std::forward<Args>(removed_content)...);

			return { get_new_key(), new_object_space };
		}
		else {
			pool_slot_type slot; 
			slot.pointing_indirector = indirection_index;

			objects.emplace_back(std::forward<Args>(removed_content)...);
			slots.emplace_back(std::move(slot));

			return { get_new_key(), objects.back() };
		}
	}
}
