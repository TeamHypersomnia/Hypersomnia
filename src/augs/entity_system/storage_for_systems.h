#pragma once
#include "augs/templates/for_each_std_get.h"

namespace augs {
	template<class... Systems>
	class storage_for_systems {
		template <class T>
		static void check_valid() {
			static_assert(is_one_of_v<T, Systems...>, "Unknown system type!");
		}

		std::tuple<Systems...> systems;
	
	public:
		template <class T>
		T& get() {
			check_valid<T>();
			return std::get<T>(systems);
		}

		template <class T>
		const T& get() const {
			check_valid<T>();
			return std::get<T>(systems);
		}

		template <class F>
		void for_each(F f) {
			for_each_through_std_get(systems, [f](auto& c) {
				f(c);
			});
		}

		template <class F>
		void for_each(F f) const {
			for_each_through_std_get(systems, [f](const auto& c) {
				f(c);
			});
		}
	};
}