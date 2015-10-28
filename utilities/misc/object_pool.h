#pragma once
#include <vector>
#include "memory_pool.h"

namespace augs {
	template<class T>
	class object_pool : private memory_pool {
		void destruct_all() {
			for (int i = 0; i < size(); ++i)
				(*this)[i].~T();
		}

	public:
		typedef typed_id<T> id;

		object_pool(int slot_count = 0) {
			initialize(slot_count);
		}

		void initialize(int slot_count) {
			memory_pool::initialize(slot_count, sizeof(T));
		}

		template<typename... Args>
		id allocate(Args... args) {
			auto raw = memory_pool::allocate();
			new (raw.ptr()) T(args...);
			return *reinterpret_cast<id*>(&raw);
		}

		bool free(id object) {
			object.get().~T();

			auto swap_elements = memory_pool::internal_free(*reinterpret_cast<memory_pool::id*>(&object));

			if (swap_elements.first == -1 && swap_elements.second == -1)
				return false;

			if (swap_elements.first != swap_elements.second) {
				int dead_offset = swap_elements.first * slot_size;
				int last_offset = swap_elements.second * slot_size;

				// move construct at the dead element's position from the last element's position
				new (pool.data() + dead_offset) T(std::move(*(T*)(pool.data() + last_offset)));
			}

			return true;
		}

		T& get(id object) { 
			return *reinterpret_cast<T*>(memory_pool::get(object)); 
		}
		
		bool alive(id object) const { 
			return memory_pool::alive(object); 
		}

		T* data() { 
			return static_cast<T*>(memory_pool::data()); 
		}

		T& operator[](int index) {
			return *reinterpret_cast<T*>(memory_pool::operator[](index));
		}

		id get_id(T* address) {
			return *reinterpret_cast<id*>(&memory_pool::get_id(reinterpret_cast<byte*>(address)));
		}

		void free_all() {
			destruct_all();
			memory_pool::free_all();
		}

		using memory_pool::size;
		using memory_pool::capacity;

		~object_pool() {
			destruct_all();
		}
	};
}