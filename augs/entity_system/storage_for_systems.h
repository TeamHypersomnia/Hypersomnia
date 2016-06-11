#pragma once
#include <tuple>
#include <memory>
#include "templates.h"

namespace detail {
	template<class T>
	struct make_unique_ptr { typedef std::unique_ptr<T> type; };
}

namespace augs {
	template<class... Systems>
	class storage_for_systems {
		typename transform_types<std::tuple, ::detail::make_unique_ptr, Systems...>::type systems;
	public:
		template <typename T>
		T& get() {
			return *std::get<std::unique_ptr<T>>(systems);
		}

		template <typename T>
		const T& get() const {
			return *std::get<std::unique_ptr<T>>(systems);
		}

		template<class S, class... Args>
		void create(Args... args) {
			std::unique_ptr<S> p(new S(args...));
			std::get<std::unique_ptr<S>>(systems) = std::move(p);
		}
	};
}