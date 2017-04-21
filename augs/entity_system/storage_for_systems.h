#pragma once
#include "augs/templates/for_each_in_types.h"

namespace augs {
	template<class... Systems>
	class storage_for_systems {
		std::tuple<Systems...> systems;

		template <typename T>
		static void check_valid() {
			static_assert(is_one_of_v<T, Systems...>, "Unknown system type!");
		}

	public:
		template <typename T>
		T& get() {
			check_valid<T>();
			return std::get<T>(systems);
		}

		template <typename T>
		const T& get() const {
			check_valid<T>();
			return std::get<T>(systems);
		}

		template <typename Pred>
		void for_each(Pred f) {
			for_each_through_std_get(systems, [f](auto& c) {
				f(c);
			});
		}

		template <typename Pred>
		void for_each(Pred f) const {
			for_each_through_std_get(systems, [f](const auto& c) {
				f(c);
			});
		}
	};
}