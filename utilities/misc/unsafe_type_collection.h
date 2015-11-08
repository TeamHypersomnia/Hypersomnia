#pragma once
#include <unordered_map>
#include <functional>

namespace augs {
	class unsafe_type_collection {
	protected:
		std::unordered_map<size_t, void*> raw_map;
	public:
		static std::unordered_map<size_t, std::function<void(void*)>> destructors;
		static std::unordered_map<size_t, std::function<void(void*, void*)>> move_constructors;

		template <typename T>
		static void register_destructor() {
			destructors[typeid(T).hash_code()] = [](void* ptr) {
				((T*)ptr)->~T();
			};
		}

		template <typename T>
		static void register_move() {
			move_constructors[typeid(T).hash_code()] = [](void* target, void* from) {
				new (target) T(std::move(*(T*)(from)));
			};
		}

		template <typename T>
		static void register_type() {
			register_destructor<T>();
			register_move<T>();
		}

		unsafe_type_collection& operator=(const unsafe_type_collection&) = delete;
		unsafe_type_collection(const unsafe_type_collection&) = delete;
		unsafe_type_collection(unsafe_type_collection&&) = delete;

		unsafe_type_collection() {}
		~unsafe_type_collection() {
			for (auto& it : raw_map)
				destructors[it.first](it.second);
		}

		template <typename T, typename... Args>
		void add(Args... args) {
			if (!find<T>())
				raw_map[typeid(T).hash_code()] = new T(args...);
		}

		template <typename T>
		void remove() {
			auto it = raw_map.find(typeid(T).hash_code());

			if (it != raw_map.end()) {
				delete reinterpret_cast<T*>((*it).second);
				raw_map.erase(it);
			}
		}

		void* find(size_t hash) {
			auto it = raw_map.find(hash);

			if (it != raw_map.end())
				return (*it).second;

			return nullptr;
		}

		template <typename T>
		T* find() {
			return reinterpret_cast<T*>(find(typeid(T).hash_code()));
		}

		void remove(size_t type_hash) {
			auto it = raw_map.find(type_hash);

			if (it != raw_map.end()) {
				destructors[type_hash]((*it).second);
				raw_map.erase(it);
			}
		}

		template <typename T>
		T& get() {
			return *find<T>();
		}

		size_t size() const {
			return raw_map.size();
		}
	};

	template <template<typename...> class Container>
	class unsafe_container_collection : private unsafe_type_collection {
		std::unordered_map<size_t, size_t> value_type_mapper;

	public:
		unsafe_container_collection() {}

		template <typename V>
		static void register_type() {
			unsafe_type_collection::register_type<Container<V>>();
		}
		
		template <typename V>
		static void register_destructor() {
			unsafe_type_collection::register_destructor<Container<V>>();
		}

		template <typename V>
		static void register_move() {
			unsafe_type_collection::register_move<Container<V>>();
		}
		
		template <typename V>
		Container<V>* find() {
			return unsafe_type_collection::find<Container<V>>();
		}

		template <typename V>
		Container<V>& get() {
			return unsafe_type_collection::get<Container<V>>();
		}
		
		template <typename V, typename... Args>
		void add(Args... args) {
			unsafe_type_collection::add<Container<V>>(args...);
			value_type_mapper[typeid(V).hash_code()] = typeid(Container<V>).hash_code();
		}

		void* find(size_t hash) {
			if (value_type_mapper.find(hash) != value_type_mapper.end())
				hash = value_type_mapper[hash];
			return unsafe_type_collection::find(hash);
		}

		void remove(size_t hash) {
			if (value_type_mapper.find(hash) != value_type_mapper.end())
				hash = value_type_mapper[hash];

			unsafe_type_collection::remove(hash);
		}

		using unsafe_type_collection::size;
		using unsafe_type_collection::~unsafe_type_collection;
	};
}


