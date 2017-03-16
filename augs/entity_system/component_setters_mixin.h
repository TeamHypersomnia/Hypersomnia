#pragma once

namespace augs {
	template<bool is_const, class derived_entity_handle>
	class component_setters_mixin {
	public:
		template <
			class component, 
			bool _is_const = is_const,
			class = std::enable_if_t<!_is_const>
		>
		decltype(auto) set(const component& c) const {
			auto& self = *static_cast<const derived_entity_handle*>(this);
			
			if (self.template has<component>()) {
				return self.template get<component>() = c;
			}
			else {
				return self.add(c);
			}
		}

		template<
			class component, 
			bool _is_const = is_const,
			class = std::enable_if_t<!_is_const>
		>
		decltype(auto) operator+=(const component& c) const {
			auto& self = *static_cast<const derived_entity_handle*>(this);
			return self.add(c);
		}

		template<
			class... added_components, 
			bool _is_const = is_const,
			class = std::enable_if_t<!_is_const>
		>
		void set(added_components... args) const {
			auto components_tuple = std::make_tuple(args...);

			for_each_in_tuple(components_tuple, [this](auto& c) {
				set(c);
			});
		}
	};
}