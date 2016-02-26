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
		typedef typed_id_template<T> typed_id;

		object_pool(int slot_count = 0) {
			initialize(slot_count);
		}

		void initialize(int slot_count) {
			memory_pool::initialize(slot_count, sizeof(T));
		}

		template<typename... Args>
		typed_id allocate(Args... args) {
			auto raw_id = allocate_with_constructor<T>(args...);
			return *reinterpret_cast<typed_id*>(&raw_id);
		}

		bool free(typed_id object) {
			object.get().~T();

			auto swap_elements = memory_pool::internal_free(*reinterpret_cast<id*>(&object));

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

		T& get(typed_id object) {
			return *reinterpret_cast<T*>(memory_pool::get(object)); 
		}
		
		bool alive(typed_id object) const {
			return memory_pool::alive(object); 
		}

		T* data() { 
			return reinterpret_cast<T*>(memory_pool::data());
		}

		T& operator[](int index) {
			return *reinterpret_cast<T*>(memory_pool::operator[](index));
		}

		typed_id get_id(T* address) {
			return *reinterpret_cast<typed_id*>(&memory_pool::get_id(reinterpret_cast<byte*>(address)));
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