#pragma once
#include <vector>
#include "ensure.h"
#include "object_pool_handle.h"
#include "object_pool_handlizer.h"

#define USE_NAMES_FOR_IDS

namespace augs {
	template<class T, typename... meta>
	class object_pool : public object_pool_handlizer<object_pool<T, meta...>> {
		typedef augs::object_pool_id<T> id_type;

		typedef augs::object_pool_handle<T, meta...> handle_type;
		typedef augs::const_object_pool_handle<T, meta...> const_handle_type;

		struct metadata {
			int pointing_indirector = -1;
		};

		struct indirector {
			int real_index = -1;
			int version = 0;
		};

		std::vector<T> pool;
		std::vector<std::tuple<meta...>> metas;
		std::vector<metadata> slots;
		std::vector<indirector> indirectors;
		std::vector<int> free_indirectors;

	public:

		object_pool(int slot_count = 0) {
			initialize_space(slot_count);
		}

		void initialize_space(int slot_count) {
			pool.clear();
			metas.clear();
			indirectors.clear();
			slots.clear();
			free_indirectors.clear();

			pool.reserve(slot_count);
			metas.reserve(slot_count);
			slots.reserve(slot_count);

			indirectors.resize(slot_count);

			free_indirectors.resize(slot_count);
			for (int i = 0; i < slot_count; ++i)
				free_indirectors[i] = i;
		}

		template<typename... Args>
		handle_type allocate(Args... args) {
			if (free_indirectors.empty())
				throw std::runtime_error("Pool is full!");

			int next_free_indirection = free_indirectors.back();
			free_indirectors.pop_back();
			indirector& indirector = indirectors[next_free_indirection];

			int new_slot_index = size();

			metadata new_slot;
			new_slot.pointing_indirector = next_free_indirection;
			indirector.real_index = new_slot_index;

			id_type allocated_id;
			allocated_id.version = indirector.version;
			allocated_id.indirection_index = next_free_indirection;

			slots.push_back(new_slot);
			pool.emplace_back(args...);
			metas.emplace_back(std::tuple<meta...>());

			return get_handle(allocated_id);
		}

		bool free(id_type object) {
			if (!alive(object))
				return false;

			int dead_index = indirectors[object.indirection_index].real_index;

			// add dead object's indirector to the free indirection list
			free_indirectors.push_back(slots[dead_index].pointing_indirector);

			// therefore we must increase version of the dead indirector
			++indirectors[object.indirection_index].version;

			if (dead_index != size() - 1) {
				int indirector_of_last_element = slots[size() - 1].pointing_indirector;

				// change last element's indirector - set it to the dead element's index
				indirectors[indirector_of_last_element].real_index = dead_index;

				slots[dead_index] = std::move(slots[size() - 1]);
				pool[dead_index] = std::move(pool[size() - 1]);
				metas[dead_index] = std::move(metas[size() - 1]);
			}

			slots.pop_back();
			pool.pop_back();
			metas.pop_back();

			return true;
		}

		handle_type get_handle(id_type from_id) {
			return{ *this, from_id };
		}

		const_handle_type get_handle(id_type from_id) const {
			return{ *this, from_id };
		}

		T& get(id_type object) {
			ensure(alive(object));
			return pool[indirectors[object.indirection_index].real_index];
		}

		const T& get(id_type object) const {
			ensure(alive(object));
			return pool[indirectors[object.indirection_index].real_index];
		}

		template <typename M>
		M& get_meta(id_type object) {
			ensure(alive(object));
			return std::get<M>(metas[indirectors[object.indirection_index].real_index]);
		}

		template <typename M>
		const M& get_meta(id_type object) const {
			ensure(alive(object));
			return std::get<M>(metas[indirectors[object.indirection_index].real_index]);
		}

		bool alive(id_type object) const {
			return object.indirection_index >= 0 && indirectors[object.indirection_index].version == object.version;
		}

		object_pool& get_pool(id_type) {
			return *this;
		}

		const object_pool& get_pool(id_type) const {
			return *this;
		}

		T* data() {
			return pool.data();
		}

		const T* data() const {
			return pool.data();
		}

		int size() const {
			return slots.size();
		}

		int capacity() const {
			return slots.capacity();
		}
	};

	template<class T>
	struct make_object_pool { typedef object_pool<T> type; };
}