#pragma once
#include <tuple>
#include <bitset>
#include "ensure.h"

namespace augs {
	template <class... components>
	class configurable_components {
		typedef std::bitset<sizeof...(components)> bitset_type;

		std::tuple<components...> data;
		bitset_type enabled_components;

		template<class component>
		typename bitset_type::reference flag() {
			return enabled_components[index_in_tuple<component, decltype(data)>::value];
		}
	public:
		template<class component>
		bool is_set() const {
			return enabled_components[index_in_tuple<component, decltype(data)>::value];
		}

		template<class component>
		component& set(const component& c = component()) {
			flag<component>() = true;
			get<component>() = c;
			return get<component>();
		}

		template<class... components>
		void set(components... args) {
			auto components_tuple = std::make_tuple(args...);

			for_each_type<components...>([this, &components_tuple](auto c) {
				set(std::get<decltype(c)>(components_tuple));
			});
		}

		template<class component>
		component& operator+=(const component& c) {
			return set(c);
		}

		template<class component>
		void remove(const component& = component()) {
			ensure(is_set<component>());
			flag<component>() = false;
		}

		template<class component>
		component& get() {
			ensure(is_set<component>());
			return std::get<component>(data);
		}

		template<class component>
		const component& get() const {
			ensure(is_set<component>());
			return std::get<component>(data);
		}
	};
}