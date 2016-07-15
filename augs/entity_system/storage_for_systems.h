#pragma once
#include <tuple>

namespace augs {
	template<class... Systems>
	class storage_for_systems {
		std::tuple<Systems...> systems;
	public:
		template <typename T>
		T& get() {
			return std::get<T>(systems);
		}

		template <typename T>
		const T& get() const {
			return std::get<T>(systems);
		}

		template <typename Pred>
		void for_each(Pred f) {
			for_each_type<Systems...>([this, f](auto c) {
				f(get<decltype(c)>());
			});
		}
	};
}