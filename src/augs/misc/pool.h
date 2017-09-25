#pragma once
#include <vector>
#include "augs/ensure.h"

#include "augs/build_settings/platform_defines.h"

#include "augs/misc/pool_handle.h"
#include "augs/misc/subscript_operator_for_get_handle_mixin.h"

namespace augs {
	struct introspection_access;

	/*	
		We generate introspectors for the pool and its internal structures,
		becasuse it will be very useful to be able to dump the values to lua files,
		if some debugging is necessary.
	*/

	struct pool_metadata {
		// GEN INTROSPECTOR struct augs::pool_metadata
		std::size_t pointing_indirector = -1;
		// END GEN INTROSPECTOR
	};

	struct pool_indirector {
		// GEN INTROSPECTOR struct augs::pool_indirector
		unsigned real_index = 0;
		unsigned version = 1;
		// END GEN INTROSPECTOR
	};

	template <class T>
	class EMPTY_BASES pool : public subscript_operator_for_get_handle_mixin<pool<T>> {
	public:
		using id_type = pooled_object_id<T>;
		using key_type = id_type;
		using unversioned_id_type = unversioned_id<T>;
		using handle_type = handle_for_pool_container<false, pool<T>, T>;
		using const_handle_type = handle_for_pool_container<true, pool<T>, T>;

		using element_type = T;
	
	protected:
		friend struct introspection_access;

		// GEN INTROSPECTOR class augs::pool class T
		std::vector<T> pooled;
		std::vector<pool_metadata> slots;
		std::vector<pool_indirector> indirectors;
		std::vector<std::size_t> free_indirectors;
		// END GEN INTROSPECTOR

	public:
		pool(const std::size_t slot_count = 0u) {
			reserve(slot_count);
		}

		void reserve(const std::size_t new_capacity) {
			const auto old_capacity = capacity();

			if (new_capacity <= old_capacity) {
				return;
			}

			pooled.reserve(new_capacity);
			slots.reserve(new_capacity);

			indirectors.resize(new_capacity);

			free_indirectors.reserve(new_capacity);

			for (std::size_t i = old_capacity; i < new_capacity; ++i) {
				free_indirectors.push_back(i);
			}
		}

		template <
			std::size_t expansion_mult = 2, 
			std::size_t expansion_add = 1, 
			class... Args
		>
		id_type allocate(Args&&... args) {
			if (full()) {
				reserve(size() * expansion_mult + expansion_add);
			}

			const auto next_free_indirector = free_indirectors.back();
			free_indirectors.pop_back();
			
			const std::size_t new_slot_index = size();

			pool_indirector& allocated_indirector = indirectors[next_free_indirector];
			allocated_indirector.real_index = new_slot_index;

			pool_metadata allocated_slot;
			allocated_slot.pointing_indirector = next_free_indirector;

			id_type allocated_id;
			allocated_id.version = allocated_indirector.version;
			allocated_id.indirection_index = next_free_indirector;

			slots.push_back(allocated_slot);
			pooled.emplace_back(std::forward<Args>(args)...);

			return allocated_id;
		}

		bool free(const id_type object) {
			if (!alive(object)) {
				return false;
			}

			const auto dead_index = get_real_index(object);

			// add dead object's indirector to the list of free indirectors
			free_indirectors.push_back(object.indirection_index);

			// therefore we must increase version of the dead indirector
			++indirectors[object.indirection_index].version;

			if (dead_index != size() - 1) {
				const auto indirector_of_last_element = slots.back().pointing_indirector;

				// change last element's indirector - set it to the dead element's index
				indirectors[indirector_of_last_element].real_index = dead_index;

				slots[dead_index] = std::move(slots.back());
				pooled[dead_index] = std::move(pooled.back());
			}

			slots.pop_back();
			pooled.pop_back();

			return true;
		}

		id_type make_versioned(const unversioned_id_type unv) const {
			id_type ver;
			ver.indirection_index = unv.indirection_index;
			ver.version = indirectors[unv.indirection_index].version;
			return ver;
		}

		handle_type get_handle(const id_type from_id) {
			return{ *this, from_id };
		}

		const_handle_type get_handle(const id_type from_id) const {
			return{ *this, from_id };
		}

		template<class Pred>
		void for_each_object_and_id(Pred f) {
			id_type id;

			for (std::size_t i = 0; i < size(); ++i) {
				const auto& s = slots[i];
				id.indirection_index = s.pointing_indirector;
				id.version = indirectors[s.pointing_indirector].version;

				f(pooled[i], id);
			}
		}

		template<class Pred>
		void for_each_object_and_id(Pred f) const {
			id_type id;

			for (std::size_t i = 0; i < size(); ++i) {
				const auto& s = slots[i];
				id.indirection_index = s.pointing_indirector;
				id.version = indirectors[s.pointing_indirector].version;

				f(pooled[i], id);
			}
		}

		template<class Pred>
		void for_each_id(Pred f) {
			id_type id;

			for (std::size_t i = 0; i < size(); ++i) {
				const auto& s = slots[i];
				id.indirection_index = s.pointing_indirector;
				id.version = indirectors[s.pointing_indirector].version;

				f(id);
			}
		}

		template<class Pred>
		void for_each_id(Pred f) const {
			id_type id;

			for (std::size_t i = 0; i < size(); ++i) {
				const auto& s = slots[i];
				id.indirection_index = s.pointing_indirector;
				id.version = indirectors[s.pointing_indirector].version;

				f(id);
			}
		}

		template<class Pred>
		void for_each(Pred f) {
			std::for_each(pooled.begin(), pooled.end(), f);
		}

		template<class Pred>
		void for_each(Pred f) const {
			std::for_each(pooled.begin(), pooled.end(), f);
		}
		
		auto get_real_index(const id_type obj) const {
			return indirectors[obj.indirection_index].real_index;
		}

		T& get(const id_type object) {
			ensure(alive(object));
			return pooled[get_real_index(object)];
		}

		const T& get(const id_type object) const {
			ensure(alive(object));
			return pooled[get_real_index(object)];
		}

		T* find(const id_type object) {
			return alive(object) ? &get(object) : nullptr; 
		}

		const T* find(const id_type object) const {
			return alive(object) ? &get(object) : nullptr; 
		}

		bool alive(const id_type object) const {
			return 
				object.indirection_index < indirectors.size() 
				&& indirectors[object.indirection_index].version == object.version
			;
		}

		const auto& get_pooled() const {
			return pooled;
		}

		T* data() {
			return pooled.data();
		}

		const T* data() const {
			return pooled.data();
		}

		auto size() const {
			return slots.size();
		}

		auto capacity() const {
			return indirectors.size();
		}

		bool empty() const {
			return size() == 0;
		}

		bool full() const {
			return size() == capacity();
		}

		template <class Archive>
		void write_object(Archive& ar) const {
			augs::write_with_capacity(ar, pooled);
			augs::write_with_capacity(ar, slots);
			augs::write_with_capacity(ar, indirectors);
			augs::write_with_capacity(ar, free_indirectors);
		}

		template <class Archive>
		void read_object(Archive& ar) {
			augs::read_with_capacity(ar, pooled);
			augs::read_with_capacity(ar, slots);
			augs::read_with_capacity(ar, indirectors);
			augs::read_with_capacity(ar, free_indirectors);
		}
	};

	template <class T>
	struct make_pool { using type = pool<T>; };
}

namespace augs {
	template<class A, class T, class...>
	void read_object(A& ar, pool<T>& storage) {
		storage.read_object(ar);
	}
	
	template<class A, class T, class...>
	void write_object(A& ar, const pool<T>& storage) {
		storage.write_object(ar);
	}
}
