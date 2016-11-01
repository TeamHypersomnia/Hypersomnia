#pragma once
#include "augs/ensure.h"
#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool is_const, class derived>
	class component_allocators_mixin {
	public:
		template<class component>
		typename maybe_const_ptr<is_const, component>::type find() const {
			auto& self = *static_cast<const derived*>(this);

			auto& aggregate = self.get();

			auto& component_pool = self.owner.get_component_pool<component>();
			auto component_id = aggregate.template get_id<component>();

			if (component_pool.alive(component_id))
				return &component_pool.get(component_id);

			return nullptr;
		}

		template<class component>
		bool has() const {
			return find<component>() != nullptr;
		}

		template<class component>
		maybe_const_ref_t<is_const, component> get() const {
			ensure(has<component>());
			return *find<component>();
		}

		template<class component, bool _is_const = is_const,
			class = std::enable_if<!_is_const>>
		  void add(const component& c) const {
			auto& self = *static_cast<const derived*>(this);
			ensure(!has<component>());
			self.get().template writable_id<component>() = self.get_cosmos().template get_component_pool<component>().allocate(c);
		}

		template<class component, bool _is_const = is_const,
			class = std::enable_if_t<!_is_const>>
			void remove() const {
			ensure(has<component>());
			auto& self = *static_cast<const derived*>(this);
			self.get_cosmos().get_component_pool<component>().free(self.get().template get_id<component>());
		}
	};
}
