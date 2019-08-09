#pragma once
#include <optional>
#include "augs/templates/traits/container_traits.h"

namespace augs {
	template <class Container>
	class simple_id_pool {
		using id_type = typename Container::value_type;

		Container ids;
	public:
		static_assert(has_constexpr_max_size_v<Container>);

		simple_id_pool() {
			reset();
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

		void reset() {
			const auto max_size = ids.max_size();
			ids.resize(max_size);

			for (std::size_t i = 0; i < max_size; ++i) {
				ids[i] = i;
			}
		}

		std::size_t free_count() const {
			return ids.size();
		}
	};
}
