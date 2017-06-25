#pragma once
#include <vector>
#include "augs/ensure.h"

#include "augs/build_settings/setting_empty_bases.h"

#include "augs/misc/pool_handle.h"
#include "augs/misc/subscript_operator_for_get_handle_mixin.h"

namespace augs {
	template<class T>
	class EMPTY_BASES pool_base : public subscript_operator_for_get_handle_mixin<pool_base<T>> {
	public:
		using id_type = pooled_object_id<T>;
		using unversioned_id_type = unversioned_id<T>;
		using handle_type = handle_for_pool_container<false, pool_base<T>, T>;
		using const_handle_type = handle_for_pool_container<true, pool_base<T>, T>;

		using pooled_container_type = std::vector<T>;

		using element_type = T;
	
	protected:
		struct metadata {
			int pointing_indirector = -1;
		};

		struct indirector {
			unsigned real_index = 0;
			unsigned version = 1;
		};

		pooled_container_type pooled;
		std::vector<metadata> slots;
		std::vector<indirector> indirectors;
		std::vector<int> free_indirectors;

		template<class F>
		bool internal_free(
			const id_type object, 
			F custom_mover
		) {
			if (!alive(object)) {
				return false;
			}

			const unsigned dead_index = get_real_index(object);

			// add dead object's indirector to the free indirection list
			free_indirectors.push_back(slots[dead_index].pointing_indirector);

			// therefore we must increase version of the dead indirector
			++indirectors[object.indirection_index].version;

			if (dead_index != size() - 1) {
				int indirector_of_last_element = slots[size() - 1].pointing_indirector;

				// change last element's indirector - set it to the dead element's index
				indirectors[indirector_of_last_element].real_index = dead_index;

				slots[dead_index] = std::move(slots[size() - 1]);
				pooled[dead_index] = std::move(pooled[size() - 1]);
				custom_mover(dead_index, size() - 1);
			}

			slots.pop_back();
			pooled.pop_back();

			return true;
		}

	public:
		pool_base(const std::size_t slot_count = 0u) {
			initialize_space(slot_count);
		}

		void initialize_space(const std::size_t slot_count) {
			pooled.clear();
			indirectors.clear();
			slots.clear();
			free_indirectors.clear();

			pooled.reserve(slot_count);
			slots.reserve(slot_count);

			indirectors.resize(slot_count);

			free_indirectors.resize(slot_count);

			for (std::size_t i = 0; i < slot_count; ++i) {
				free_indirectors[i] = i;
			}
		}

		template <typename... Args>
		id_type allocate(Args&&... args) {
			if (free_indirectors.empty()) {
				throw std::runtime_error("Pool is full!");
			}

			const int next_free_indirector = free_indirectors.back();
			free_indirectors.pop_back();

			indirector& indirector = indirectors[next_free_indirector];

			const std::size_t new_slot_index = size();

			metadata new_slot;
			new_slot.pointing_indirector = next_free_indirector;
			indirector.real_index = new_slot_index;

			id_type allocated_id;
			allocated_id.version = indirector.version;
			allocated_id.indirection_index = next_free_indirector;

			slots.push_back(new_slot);
			pooled.push_back(T(std::forward<Args>(args)...));

			return allocated_id;
		}

		bool free(const id_type object) {
			return internal_free(object, [](auto...) {});
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
				const metadata& s = slots[i];
				id.indirection_index = s.pointing_indirector;
				id.version = indirectors[s.pointing_indirector].version;

				f(pooled[i], id);
			}
		}

		template<class Pred>
		void for_each_object_and_id(Pred f) const {
			id_type id;

			for (std::size_t i = 0; i < size(); ++i) {
				const metadata& s = slots[i];
				id.indirection_index = s.pointing_indirector;
				id.version = indirectors[s.pointing_indirector].version;

				f(pooled[i], id);
			}
		}
		template<class Pred>
		void for_each_id(Pred f) {
			id_type id;

			for (std::size_t i = 0; i < size(); ++i) {
				const metadata& s = slots[i];
				id.indirection_index = s.pointing_indirector;
				id.version = indirectors[s.pointing_indirector].version;

				f(id);
			}
		}

		template<class Pred>
		void for_each_id(Pred f) const {
			id_type id;

			for (std::size_t i = 0; i < size(); ++i) {
				const metadata& s = slots[i];
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
		
		unsigned get_real_index(const id_type obj) const {
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
			return object.indirection_index >= 0 && indirectors[object.indirection_index].version == object.version;
		}

		const pooled_container_type& get_pooled() const {
			return pooled;
		}

		T* data() {
			return pooled.data();
		}

		const T* data() const {
			return pooled.data();
		}

		std::size_t size() const {
			return slots.size();
		}

		std::size_t capacity() const {
			return indirectors.size();
		}

		bool empty() const {
			return size() == 0;
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
	class pool : public pool_base<T> {
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
