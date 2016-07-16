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
				storage_for_systems& self = *this;

				auto& sys = self.get<decltype(c)>();
				f(sys);
			});
		}

		template <typename Pred>
		void for_each(Pred f) const {
			for_each_type<Systems...>([this, f](auto c) {
				const storage_for_systems& self = *this;

				auto& sys = self.get<decltype(c)>();
				f(sys);
			});
		}
	};
}