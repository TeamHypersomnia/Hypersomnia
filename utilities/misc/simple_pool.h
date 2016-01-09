#pragma once
#include <vector>

namespace augs {
	template<class T>
	class simple_pool {
		struct entry {
			bool active;
			char obj[sizeof(T)];
		};

		std::vector<entry> pool;

	public:
		template <class... Args>
		int allocate(Args... args) {
			for (int i = 0; i < pool.size(); ++i) {
				auto& e = pool[i];

				if (!e.active) {
					e.active = true;
					new ((T*)(&e.obj[0])) T(args...);
					return i;
				}
			}

			entry new_entry;
			new_entry.active = true;

			pool.push_back(new_entry);
			new ((T*)((*pool.rbegin()).obj)) T(args...);

			return pool.size()-1;
		}

		T& get(int ind) {
			return *((T*)pool[ind].obj);
		}

		void destroy(int ind) {
			pool[ind].active = false;
			((T*)pool[ind].obj)->~T();
		}
	};
}