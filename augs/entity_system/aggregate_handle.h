#pragma once
#include "misc/object_pool_handle.h"
#include "component_aggregate.h"

namespace augs {
	template <bool is_const, class owner_type, class T>
	class basic_aggregate_handle;

	template <bool is_const, class owner_type, class... components>
	class basic_aggregate_handle<is_const, owner_type, std::tuple<components...>> 
		: public basic_handle<is_const, owner_type, component_aggregate<components...>> {
		using basic_handle::get;
	public:
		using basic_handle::basic_handle;

		template<class component>
		typename std::conditional<is_const, const component*, component*>::type find() const {
			auto& aggregate = get();

			auto& component_pool = owner.get_component_pool<component>();
			auto component_handle = component_pool.get_handle(aggregate.get_id<component>());

			if (component_handle.alive())
				return &component_handle.get();

			return nullptr;
		}

		template<class component>
		typename std::conditional<is_const, const component&, component&>::type get() const {
			return *find<component>();
		}

		template<class component>
		bool has() const {
			return find<component>() != nullptr;
		}

		template<class component,
			class = typename std::enable_if<!is_const>::type>
			component& set(const component& c) const {
			get<component>() = c;
			return get<component>();
		}

		template<class... components,
			class = typename std::enable_if<!is_const>::type>
			void set(components... args) const {
			auto components_tuple = std::make_tuple(args...);

			for_each_type<components...>([this, &components_tuple](auto c) {
				set(std::get<decltype(c)>(components_tuple));
			});
		}

		template<class component,
			class = typename std::enable_if<!is_const>::type>
			component& add(const component& c) {
			get().writable_id<component>() = owner.get_component_pool<component>().allocate_component(c);
		}

		template<class component,
			class = typename std::enable_if<!is_const>::type>
		component& operator+=(const component& c) {
			return add(c);
		}

		template<class component>
		typename std::enable_if<!is_const, component&>::type 
			set(const component& c = component()) {
			get<component>() = c;
			return get<component>();
		}
	};
}