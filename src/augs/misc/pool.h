#pragma once
#include "augs/ensure.h"
#include "augs/misc/pooled_object_id.h"

namespace augs {
	struct introspection_access;

	/*	
		We generate introspectors for the pool and its internal structures,
		becasuse it will be very useful to be able to dump the values to lua files,
		if some debugging is necessary.
	*/

	struct pool_slot {
		// GEN INTROSPECTOR struct augs::pool_slot
		unsigned pointing_indirector = -1;
		// END GEN INTROSPECTOR
	};

	struct pool_indirector {
		// GEN INTROSPECTOR struct augs::pool_indirector
		unsigned real_index = 0;
		unsigned version = 1;
		// END GEN INTROSPECTOR
	};

	template <class mapped_type, template <class> class make_container_type>
	class pool {
	public:
		using key_type = pooled_object_id<mapped_type>;
		using unversioned_id_type = unversioned_id<mapped_type>;

	protected:
		using size_type = unsigned;

		friend struct introspection_access;

		// GEN INTROSPECTOR class augs::pool class mapped_type template<class>class C
		make_container_type<pool_slot> slots;
		make_container_type<mapped_type> objects;
		make_container_type<pool_indirector> indirectors;
		make_container_type<size_type> free_indirectors;
		// END GEN INTROSPECTOR

		auto& get_indirector(const key_type key) {
			return indirectors[key.indirection_index];
		}

		const auto& get_indirector(const key_type key) const {
			return indirectors[key.indirection_index];
		}

		bool correct_range(const key_type key) const {
			return key.indirection_index < indirectors.size();
		}

		bool versions_match(const pool_indirector& indirector, const key_type key) const {
			return indirector.version == key.version;
		}

	public:
		pool(const size_type slot_count = 0u) {
			reserve(slot_count);
		}

		void reserve(const size_type new_capacity) {
			const auto old_capacity = capacity();

			if (new_capacity <= old_capacity) {
				return;
			}

			slots.reserve(new_capacity);
			objects.reserve(new_capacity);

			indirectors.resize(new_capacity);
			free_indirectors.reserve(new_capacity);

			for (size_type i = old_capacity; i < new_capacity; ++i) {
				free_indirectors.push_back(i);
			}
		}

		template <
			size_type expansion_mult = 2, 
			size_type expansion_add = 1, 
			class... Args
		>
		key_type allocate(Args&&... args) {
			if (full()) {
				reserve(size() * expansion_mult + expansion_add);
			}

			const auto next_free_indirector = free_indirectors.back();
			free_indirectors.pop_back();
			
			const auto new_slot_index = size();

			pool_indirector& allocated_indirector = indirectors[next_free_indirector];
			allocated_indirector.real_index = new_slot_index;

			key_type allocated_id;
			allocated_id.version = allocated_indirector.version;
			allocated_id.indirection_index = next_free_indirector;

			pool_slot allocated_slot;
			allocated_slot.pointing_indirector = next_free_indirector;

			slots.push_back(allocated_slot);
			objects.emplace_back(std::forward<Args>(args)...);

			return allocated_id;
		}

		bool free(const key_type key) {
			if (!correct_range(key)) {
				return false;
			}

			auto& indirector = get_indirector(key);

			if (!versions_match(indirector, key)) {
				return false;
			}

			// add dead key's indirector to the list of free indirectors
			free_indirectors.push_back(key.indirection_index);

			// therefore we must increase version of the dead indirector
			++indirector.version;

			const auto removed_at_index = indirector.real_index;

			if (const bool need_to_move_last = removed_at_index != size() - 1) {
				const auto indirector_of_last_element = slots.back().pointing_indirector;

				// change last element's indirector - set it to the removed element's index
				indirectors[indirector_of_last_element].real_index = removed_at_index;

				slots[removed_at_index] = std::move(slots.back());
				objects[removed_at_index] = std::move(objects.back());
			}

			slots.pop_back();
			objects.pop_back();

			return true;
		}

		key_type make_versioned(const unversioned_id_type key) const {
			key_type ver;
			ver.indirection_index = key.indirection_index;
			ver.version = indirectors[key.indirection_index].version;
			return ver;
		}

		mapped_type& get(const key_type key) {
			ensure(correct_range(key));

			const auto& indirector = get_indirector(key);

			ensure(versions_match(indirector, key));

			return objects[indirector.real_index];
		}

		const mapped_type& get(const key_type key) const {
			ensure(correct_range(key));

			const auto& indirector = get_indirector(key);

			ensure(versions_match(indirector, key));

			return objects[indirector.real_index];
		}

		mapped_type* find(const key_type key) {
			if (!correct_range(key)) {
				return nullptr;
			}

			const auto& indirector = get_indirector(key);

			if (!versions_match(indirector, key)) {
				return nullptr;
			}

			return &objects[indirector.real_index];
		}

		const mapped_type* find(const key_type key) const {
			if (!correct_range(key)) {
				return nullptr;
			}

			const auto& indirector = get_indirector(key);

			if (!versions_match(indirector, key)) {
				return nullptr;
			}

			return &objects[indirector.real_index];
		}

		bool alive(const key_type key) const {
			return correct_range(key) && versions_match(get_indirector(key), key);
		}

		bool dead(const key_type key) const {
			return !alive(key);
		}

		mapped_type* data() {
			return objects.data();
		}

		const mapped_type* data() const {
			return objects.data();
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

		template <class F>
		void for_each_object_and_id(F f) {
			key_type id;

			for (size_type i = 0; i < size(); ++i) {
				const auto& s = slots[i];
				id.indirection_index = s.pointing_indirector;
				id.version = indirectors[s.pointing_indirector].version;

				f(objects[i], id);
			}
		}

		template <class F>
		void for_each_object_and_id(F f) const {
			for (size_type i = 0; i < size(); ++i) {
				key_type id;

				const auto& s = slots[i];
				id.indirection_index = s.pointing_indirector;
				id.version = indirectors[s.pointing_indirector].version;

				f(objects[i], id);
			}
		}

		template <class F>
		void for_each_id(F f) {
			for (size_type i = 0; i < size(); ++i) {
				key_type id;

				const auto& s = slots[i];
				id.indirection_index = s.pointing_indirector;
				id.version = indirectors[s.pointing_indirector].version;

				f(id);
			}
		}

		template <class F>
		void for_each_id(F f) const {
			for (size_type i = 0; i < size(); ++i) {
				key_type id;

				const auto& s = slots[i];
				id.indirection_index = s.pointing_indirector;
				id.version = indirectors[s.pointing_indirector].version;

				f(id);
			}
		}

		auto begin() {
			return objects.begin();
		}

		auto begin() const {
			return objects.begin();
		}

		auto end() {
			return objects.end();
		}

		auto end() const {
			return objects.end();
		}

		template <class Archive>
		void write_object(Archive& ar) const {
			augs::write_with_capacity(ar, objects);
			augs::write_with_capacity(ar, slots);
			augs::write_with_capacity(ar, indirectors);
			augs::write_with_capacity(ar, free_indirectors);
		}

		template <class Archive>
		void read_object(Archive& ar) {
			augs::read_with_capacity(ar, objects);
			augs::read_with_capacity(ar, slots);
			augs::read_with_capacity(ar, indirectors);
			augs::read_with_capacity(ar, free_indirectors);
		}
	};
}

namespace augs {
	template <class A, class mapped_type, template <class> class C>
	void read_object(A& ar, pool<mapped_type, C>& storage) {
		storage.read_object(ar);
	}
	
	template <class A, class mapped_type, template <class> class C>
	void write_object(A& ar, const pool<mapped_type, C>& storage) {
		storage.write_object(ar);
	}
}
