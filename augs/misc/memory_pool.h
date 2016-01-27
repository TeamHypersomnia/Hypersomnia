#pragma once
#include <vector>
#include "simple_pool.h"
#define USE_NAMES_FOR_IDS

namespace augs {
	template<class T> class object_pool;

	class memory_pool {
		static simple_pool<memory_pool*> pool_locations;

	protected:
		typedef char byte;

		struct pool_id {
			int pointer_id = -1;
			void unset();

			memory_pool* operator->() const;
			memory_pool& operator*() const;
			operator bool() const;
		};

		pool_id this_pool_pointer_location;

		int slot_size;
		int count = 0;
		
		std::vector<byte> pool;

		struct metadata {
			int pointing_indirector = -1;
		};

		std::vector<metadata> slots;

		struct indirector {
			int real_index = -1;
			int version = 0;

			operator int() {
				return real_index;
			}
		};

		std::vector<indirector> indirectors;

		std::vector<int> free_indirectors;

		memory_pool(const memory_pool&) = delete;
		memory_pool& operator=(const memory_pool&) = delete;

	public:
		class id {
		protected:
			friend class memory_pool;

			pool_id owner;
			int version = 0xdeadbeef;
			int indirection_index = 0xdeadbeef;
			
		public:
			id();

			bool operator<(const id&) const;
			bool operator!() const;
			bool operator!=(const id&) const;
			bool operator==(const id&) const;

			byte* ptr();
			const byte* ptr() const;
			memory_pool& get_pool();

			bool alive() const;
			bool dead() const;

			void unset();
#ifdef USE_NAMES_FOR_IDS
			char name[30];
#endif
		};

		template <typename T>
		class typed_id : private id {
		public:
			typed_id() {}

			const T& get() const { return *reinterpret_cast<T*>(id::ptr()); }
			const T* ptr() const { return  reinterpret_cast<T*>(id::ptr()); }

			T* ptr() { return  reinterpret_cast<T*>(id::ptr()); }
			T& get() { return *reinterpret_cast<T*>(id::ptr()); }

			object_pool<T>& get_pool() { return reinterpret_cast<object_pool<T>&>(id::get_pool()); }
			T& operator*() { return get(); }
			T* operator->() { return &get(); }

			using id::operator!;
			bool operator< (const typed_id& b) const { return id::operator< (reinterpret_cast<const id&>(b)); }
			bool operator==(const typed_id& b) const { return id::operator==(reinterpret_cast<const id&>(b)); }
			bool operator!=(const typed_id& b) const { return id::operator!=(reinterpret_cast<const id&>(b)); }

			bool alive() const { return id::alive(); }
			bool dead() const { return id::dead(); }

			using id::unset;
#ifdef USE_NAMES_FOR_IDS
			using id::name;
#endif
		};

		memory_pool(int slot_count = 0, int slot_size = 0);
		~memory_pool();

		void initialize(int slot_count, int slot_size);
		void resize(int slot_count);

		id allocate();
		bool free(id);
		void free_all();

		id allocate_with_default_construct(size_t type_hash);
		bool free_with_destructor(id, size_t type_hash);
		void destruct_all(size_t type_hash);

		id get_id(byte* address);

		byte* get(id object);
		const byte* get(id object) const;

		bool alive(id object) const;

		byte* data();
		byte* operator[](int index);

		int size() const;
		int capacity() const;

	protected:
		std::pair<int, int> internal_free(id);
	};
}