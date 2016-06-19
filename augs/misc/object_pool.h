#pragma once
#include <vector>
#include "ensure.h"

#define USE_NAMES_FOR_IDS

namespace augs {
	template<class T>
	class object_pool {
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
		class id {
		public:
#ifdef USE_NAMES_FOR_IDS
			char debug_name[40];
#endif
			int version = 0xdeadbeef;
			int indirection_index = -1;

#ifdef USE_NAMES_FOR_IDS
			id() {
				set_debug_name("unset");
			}
#endif
			void unset() {
				*this = id();
			}

			void set_debug_name(std::string s) {
#ifdef USE_NAMES_FOR_IDS
				ensure(s.size() < sizeof(debug_name) / sizeof(char));
				strcpy(debug_name, s.c_str());
#endif
			}

			std::string get_debug_name() const {
#ifdef USE_NAMES_FOR_IDS
				return debug_name;
#else
				ensure(0);
#endif
			}
		};

		static id dead_id;

		template<class reference_type>
		class basic_handle {
		public:
			reference_type& owner;
			id raw_id;

			basic_handle(reference_type& owner, id raw_id) : owner(owner), raw_id(raw_id) {}

			bool alive() const {
				return owner.alive(*this);
			}

			bool dead() const {
				return !alive();
			}

			bool operator!() const {
				return !alive();
			}

			bool operator==(const id& b) const {
				bool result = alive() && b.alive() && &owner == &b.owner && (indirection_index == b.indirection_index && version == b.version);
				return result;
			}

			bool operator!=(const id& b) const {
				return !operator==(b);
			}

			const T& get() const {
				return owner.get(*this);
			}

			const object_pool& get_pool() const {
				return owner;
			}

			std::string get_debug_name() const {
				return raw_id.get_debug_name();
			}

			id get_id() const {
				return raw_id;
			}
		};

		class handle : public basic_handle<object_pool> {
		public:
			void unset() {
				raw_id.unset();
			}

			object_pool& get_pool() {
				return owner;
			}

			T& get() {
				return owner.get(*this);
			}

			void set_debug_name(std::string s) {
				raw_id.set_debug_name(s);
			}
		};

		typedef basic_handle<const object_pool> const_handle;

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
		id allocate(Args... args) {
			if (free_indirectors.empty())
				throw std::runtime_error("Pool is full!");

			int next_free_indirection = free_indirectors.back();
			free_indirectors.pop_back();
			indirector& indirector = indirectors[next_free_indirection];

			int new_slot_index = size();

			metadata new_slot;
			new_slot.pointing_indirector = next_free_indirection;
			indirector.real_index = new_slot_index;

			id allocated_id;
			allocated_id.version = indirector.version;
			allocated_id.indirection_index = next_free_indirection;

			slots.push_back(new_slot);
			pool.emplace_back(args...);

			return allocated_id;
		}

		bool free(id object) {
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

			slots.erase(slots.rbegin());
			pool.erase(slots.rbegin());
		}

		handle get_handle(id from_id) {
			return{ *this, from_id };
		}

		const_handle get_handle(id from_id) const {
			return{ *this, from_id };
		}

		T& get(id object) {
			if (!alive(object))
				return nullptr;

			return pool[indirectors[object.indirection_index]];
		}

		const T& get(id object) const {
			if (!alive(object))
				return nullptr;

			return pool[indirectors[object.indirection_index]];
		}

		bool alive(id object) const {
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