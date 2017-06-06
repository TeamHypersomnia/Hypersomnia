#pragma once

namespace augs {
	template<bool is_const, class derived_entity_handle>
	class component_setters_mixin {
	public:
		template <
			class component, 
			class = std::enable_if_t<!is_const>
		>
		void set(const component& c) const {
			auto& self = *static_cast<const derived_entity_handle*>(this);
			
			if (self.template has<component>()) {
				self.template get<component>() = c;
			}
			else {
				self.add(c);
			}
		}

		template<
			class component, 
			class = std::enable_if_t<!is_const>
		>
		decltype(auto) operator+=(const component& c) const {
			auto& self = *static_cast<const derived_entity_handle*>(this);
			return self.add(c);
		}

		template<
			class... added_components, 
			class = std::enable_if_t<!is_const>
		>
		void set(added_components... args) const {
			auto components_tuple = std::make_tuple(args...);

			for_each_through_std_get(components_tuple, [this](auto& c) {
				set(c);
			});
		}
	};
}