#pragma once
#include <cstddef>
#include <optional>

namespace augs {
	template <class Container>
	class simple_id_pool {
		using id_type = typename Container::value_type;

		Container ids;
	public:
		simple_id_pool() = default;
		
		simple_id_pool(const std::size_t max_size) {
			reset(max_size);
		}

		id_type allocate() {
			const auto last = ids.back();
			ids.pop_back();

			return last;
		}

		void free(const id_type& id) {
			ids.emplace_back(id);
		}

		bool full() const {
			return ids.empty();
		}

		void reset(const std::size_t max_size) {
			ids.resize(max_size);

			for (std::size_t i = 0; i < max_size; ++i) {
				ids[i] = max_size - i - 1;
			}
		}

		std::size_t free_count() const {
			return ids.size();
		}
	};
}
