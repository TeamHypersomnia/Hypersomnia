#pragma once
#include <tuple>
#include <bitset>

namespace augs {
	template <class... components>
	struct configurable_components {
		std::tuple<components...> data;
		std::bitset<sizeof...(components)> enabled_components;

		template<class component>
		component& add(const component& c = component()) {
			enabled_components.set(index_in_tuple<component, decltype(data)>::value, true);
			set(c);
			return get();
		}

		template<class component>
		component& operator+=(const component& c = component()) {
			return add(c);
		}

		template<class component>
		void remove(const component& = component()) {
			enabled_components.set(index_in_tuple<component, decltype(data)>::value, false);
		}

		template<class component>
		component& get() {
			return std::get<component>(data);
		}

		template<class component>
		void set(const component& c = component()) {
			get() = c;
		}

		template<class component>
		bool is_enabled() const {
			enabled_components[index_in_tuple<component, decltype(data)>::value];
		}
	};
}