#pragma once
#include <vector>
#define USE_NAMES_FOR_IDS

namespace augs {
	template<class T> class object_pool;

	class memory_pool {
	protected:
		typedef char byte;

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
		public:
#ifdef USE_NAMES_FOR_IDS
			char debug_name[40];
#endif
		protected:
			friend class memory_pool;

			int version = -1;
			int indirection_index = -1;
		};

		class handle : private id {
			memory_pool& owner;
			handle(id, memory_pool&);

			friend class memory_pool;
		public:

			bool operator<(const handle&) const;
			bool operator!() const;
			bool operator!=(const handle&) const;
			bool operator==(const handle&) const;

			byte* ptr();
			const byte* ptr() const;
			memory_pool& get_pool();

			bool alive() const;
			bool dead() const;

			void unset();

			void set_debug_name(std::string);
			std::string get_debug_name() const;
		};

		static id dead_id;

		template <typename T>
		class typed_handle_interface : private handle {
		public:
			using handle::handle;

			const T& get() const { return *reinterpret_cast<const T*>(handle::ptr()); }
			const T* ptr() const { return  reinterpret_cast<const T*>(handle::ptr()); }

			T* ptr() { return  reinterpret_cast<T*>(handle::ptr()); }
			T& get() { return *reinterpret_cast<T*>(handle::ptr()); }

			object_pool<T>& get_pool() { return reinterpret_cast<object_pool<T>&>(handle::get_pool()); }
			T& operator*() { return get(); }
			T* operator->() { return &get(); }
			const T& operator*() const { return get(); }
			const T* operator->() const { return &get(); }

			using handle::operator!;
			bool operator< (const typed_handle_interface& b) const { return handle::operator< (reinterpret_cast<const handle&>(b)); }
			bool operator==(const typed_handle_interface& b) const { return handle::operator==(reinterpret_cast<const handle&>(b)); }
			bool operator!=(const typed_handle_interface& b) const { return handle::operator!=(reinterpret_cast<const handle&>(b)); }

			bool alive() const { return handle::alive(); }
			bool dead() const { return handle::dead(); }

			using handle::unset;
			using handle::get_debug_name;
			using handle::set_debug_name;
		};

		template <typename T>
		class typed_handle_template : public typed_handle_interface<T> {

		};

		memory_pool(int slot_count = 0, int slot_size = 0);
		~memory_pool();

		template<class T>
		void associate_type_for_typed_operations() {
			associated_type_hash = typeid(T).hash_code();
			associated_type_hash_set = true;
		}

		void initialize(int slot_count, int slot_size);
		void resize(int slot_count);

		id allocate();

		bool free(id);
		void free_all();

		template<typename T, typename... Args>
		id allocate_with_constructor(Args... args) {
			auto raw = memory_pool::allocate();
			new (raw.ptr()) T(args...);
			return *reinterpret_cast<id*>(&raw);
		}

		bool free_with_destructor(id);
		void destruct_all();

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