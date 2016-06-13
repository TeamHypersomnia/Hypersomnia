#pragma once
#include <tuple>
#include <bitset>
#include "ensure.h"

namespace augs {
	template <class... components>
	struct configurable_components {
		std::tuple<components...> data;
		std::bitset<sizeof...(components)> enabled_components;

		template<class component>
		bool is_enabled() const {
			enabled_components[index_in_tuple<component, decltype(data)>::value];
		}

		template<class component>
		component& add(const component& c = component()) {
			enabled_components.set(index_in_tuple<component, decltype(data)>::value, true);
			set(c);
			return get<component>();
		}

		template<class component>
		component& operator+=(const component& c) {
			return add(c);
		}

		template<class component>
		void remove(const component& = component()) {
			ensure(is_enabled<component>());
			enabled_components.set(index_in_tuple<component, decltype(data)>::value, false);
		}

		template<class component>
		component& get() {
			ensure(is_enabled<component>());
			return std::get<component>(data);
		}

		template<class component>
		const component& get() const {
			ensure(is_enabled<component>());
			return std::get<component>(data);
		}

		template<class component>
		void set(const component& c = component()) {
			get<component>() = c;
		}
	};
}