#pragma once

namespace augs {
	template<bool is_const, class derived_entity_handle>
	class component_setters_mixin {
	public:
		template <
			class component,
			bool C = !is_const, class = std::enable_if_t<C>
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
			bool C = !is_const, class = std::enable_if_t<C>
		>
		decltype(auto) operator+=(const component& c) const {
			auto& self = *static_cast<const derived_entity_handle*>(this);
			self.add(c);
			return self.template get<component>();
		}
	};
}