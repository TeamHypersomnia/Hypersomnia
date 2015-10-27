#pragma once
#include <vector>

namespace augs {
	template<class T> class object_pool;

	class memory_pool {
	protected:
		typedef char byte;

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

			memory_pool* owner = nullptr;
			int version;
			int indirection_index;
			
		public:
			bool operator<(const id&) const;
			bool operator!() const;
			bool operator!=(const id&) const;
			bool operator==(const id&) const;

			byte* ptr();
			const byte* ptr() const;
			memory_pool& get_pool();

			bool alive() const;
			bool dead() const;
		};

		template <typename T>
		class typed_id : private id {
		public:
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
		};

		memory_pool(int slot_count = 0, int slot_size = 0);
		void initialize(int slot_count, int slot_size);
		
		id allocate();
		bool free(id);
		void free_all();

		id get_id(byte* address);

		byte* get(id object);
		const byte* get(id object) const;

		bool alive(id object) const;

		byte* data();
		byte* operator[](int index);

		int size() const;
		int capacity() const;
	};
}