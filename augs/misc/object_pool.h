#pragma once
#include <vector>
#include "ensure.h"
#include "object_pool_id.h"

#define USE_NAMES_FOR_IDS

namespace augs {
	template<class T>
	class object_pool {
		typedef augs::object_pool_id<T> object_pool_id;

		struct metadata {
			int pointing_indirector = -1;
		};

		struct indirector {
			int real_index = -1;
			int version = 0;
		};

		std::vector<T> pool;
		std::vector<metadata> slots;
		std::vector<indirector> indirectors;
		std::vector<int> free_indirectors;

	public:

		template<bool is_const>
		class basic_handle {
			typedef typename std::conditional<is_const, const object_pool&, object_pool&>::type pool_reference;
			typedef typename std::conditional<is_const, const T&, T&>::type value_reference;

		public:
			pool_reference owner;
			object_pool_id raw_id;

			basic_handle(pool_reference owner, object_pool_id raw_id) : owner(owner), raw_id(raw_id) {}

			void unset() {
				raw_id.unset();
			}

			void set_debug_name(std::string s) {
				raw_id.set_debug_name(s);
			}

			value_reference get() const {
				return owner.get(raw_id);
			}

			bool alive() const {
				return owner.alive(raw_id);
			}

			bool dead() const {
				return !alive();
			}

			pool_reference get_pool() const {
				return owner;
			}

			object_pool_id get_id() const {
				return raw_id;
			}

			//bool operator!() const {
			//	return !alive();
			//}
			//
			//bool operator==(const basic_handle& b) const {
			//	bool result = alive() && b.alive() && &owner == &b.owner && raw_id == b.raw_id;
			//	return result;
			//}
			//
			//bool operator!=(const basic_handle& b) const {
			//	return !operator==(b);
			//}

			std::string get_debug_name() const {
				return raw_id.get_debug_name();
			}
		};

		typedef basic_handle<false> handle;
		typedef basic_handle<true> const_handle;

		object_pool(int slot_count = 0) {
			initialize_space(slot_count);
		}

		void initialize_space(int slot_count) {
			pool.clear();
			indirectors.clear();
			slots.clear();
			free_indirectors.clear();

			pool.reserve(slot_count);
			slots.reserve(slot_count);

			indirectors.resize(slot_count);

			free_indirectors.resize(slot_count);
			for (int i = 0; i < slot_count; ++i)
				free_indirectors[i] = i;
		}

		template<typename... Args>
		object_pool_id allocate(Args... args) {
			if (free_indirectors.empty())
				throw std::runtime_error("Pool is full!");

			int next_free_indirection = free_indirectors.back();
			free_indirectors.pop_back();
			indirector& indirector = indirectors[next_free_indirection];

			int new_slot_index = size();

			metadata new_slot;
			new_slot.pointing_indirector = next_free_indirection;
			indirector.real_index = new_slot_index;

			object_pool_id allocated_id;
			allocated_id.version = indirector.version;
			allocated_id.indirection_index = next_free_indirection;

			slots.push_back(new_slot);
			pool.emplace_back(args...);

			return allocated_id;
		}

		bool free(object_pool_id object) {
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
			}

			slots.pop_back();
			pool.pop_back();

			return true;
		}

		handle get_handle(object_pool_id from_id) {
			return{ *this, from_id };
		}

		const_handle get_handle(object_pool_id from_id) const {
			return{ *this, from_id };
		}

		T& get(object_pool_id object) {
			ensure(alive(object));
			return pool[indirectors[object.indirection_index].real_index];
		}

		const T& get(object_pool_id object) const {
			ensure(alive(object));
			return pool[indirectors[object.indirection_index].real_index];
		}

		bool alive(object_pool_id object) const {
			return object.indirection_index >= 0 && indirectors[object.indirection_index].version == object.version;
		}

		T* data() {
			return pool.data();
		}

		T& operator[](int index) {
			return pool[index];
		}

		const T* data() const {
			return pool.data();
		}

		const T& operator[](int index) const {
			return pool[index];
		}

		int size() const {
			return slots.size();
		}

		int capacity() const {
			return slots.capacity();
		}
	};
}