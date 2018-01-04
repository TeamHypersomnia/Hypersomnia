#pragma once
#include "augs/build_settings/setting_debug_track_entity_name.h"
#include "augs/misc/trivially_copyable_tuple.h"
#include "augs/templates/type_matching_and_indexing.h"
#include "augs/templates/transform_types.h"
#include "augs/templates/component_traits.h"
#include "augs/pad_bytes.h"

namespace augs {
	template <template <class T> class make_pool_id, class... components>
	class component_aggregate {
		template <class component, class Aggregate, class PoolProvider>
		static auto* find_impl(Aggregate& a, PoolProvider& p) {
			if constexpr(is_component_always_present_v<component>) {
				return &std::get<component>(a.always_presents);
			}
			else {
				return p.template get_component_pool<component>().find(a.template get_id<component>());
			}
		}

		template <class component, class Aggregate, class PoolProvider>
		static auto& get_impl(Aggregate& a, PoolProvider& p) {
			if constexpr(!is_component_always_present_v<component>) {
				ensure(a.template has<component>(p));
			}

			return *find_impl<component>(a, p);
		}

		template <class F, class Aggregate, class PoolProvider>
		static void for_each_component_impl(F&& callback, Aggregate& a, PoolProvider& p) {
			auto callbacker = [&](auto c){
				using component_type = decltype(c);

				if (const auto maybe_component = a.template find<component_type>(p)) {
					callback(*maybe_component);
				}
			};

			(callbacker(components()), ...);
		}

		template <class F, class Aggregate, class PoolProvider>
		static void for_each_dynamic_component_impl(F&& callback, Aggregate& a, PoolProvider& p) {
			auto callbacker = [&](auto c){
				using component_type = decltype(c);

				if constexpr(!is_component_always_present_v<component_type>) {
					if (const auto maybe_component = a.template find<component_type>(p)) {
						callback(*maybe_component);
					}
				}
			};

			(callbacker(components()), ...);
		}

	public:
		using dynamic_components_list = filter_types<apply_negation<is_component_always_present>::template type, components...>;
		using always_present_components_list = filter_types<is_component_always_present, components...>;

		using dynamic_component_id_tuple = 
			replace_list_type_t<
				transform_types_in_list_t<
					dynamic_components_list, 
					make_pool_id
				>, 
				trivially_copyable_tuple
			>
		;

		using always_present_components_tuple = 
			replace_list_type_t<
				always_present_components_list, 
				trivially_copyable_tuple
			>
		;

		// GEN INTROSPECTOR class augs::component_aggregate template<class>class make_pool_id class... components
		always_present_components_tuple always_presents;
		dynamic_component_id_tuple component_ids;
		// END GEN INTROSPECTOR

		template <class component>
		void set_id(const make_pool_id<component> to) {
			std::get<make_pool_id<component>>(component_ids) = to;
		}

		template <class component>
		auto get_id() const {
			return std::get<make_pool_id<component>>(component_ids);
		}

		template <class component, class PoolProvider>
		auto& get(PoolProvider& p) {
			return get_impl<component>(*this, p);
		}

		template <class component, class PoolProvider>
		auto& get(const PoolProvider& p) const {
			return get_impl<component>(*this, p);
		}

		template <class component, class PoolProvider>
		auto* find(PoolProvider& p) {
			return find_impl<component>(*this, p);
		}

		template <class component, class PoolProvider>
		auto* find(const PoolProvider& p) const {
			return find_impl<component>(*this, p);
		}

		template <class component, class PoolProvider>
		bool has(PoolProvider& p) const {
			if constexpr(is_component_always_present_v<component>) {
				return true;
			}
			else {
				return find<component>(p) != nullptr;
			}
		}

		template <class component, class PoolProvider>
		void add(const component& c, PoolProvider& p) {
			if constexpr(is_component_always_present_v<component>) {
				get<component>(p) = c;
			}
			else {
				ensure(!has<component>(p));

				set_id(p.template get_component_pool<component>().allocate(c));
			}
		}

		template <class component, class PoolProvider>
		void remove(PoolProvider& p) {
			static_assert(!is_component_always_present_v<component>, "Can't remove an always_present component.");

			ensure(has<component>(p));

			const auto id_of_deleted = get_id<component>();

			p.template get_component_pool<component>().free(id_of_deleted);
			set_id(decltype(id_of_deleted)());
		}

		template <class F, class PoolProvider>
		void for_each_component(F&& callback, PoolProvider& p) {
			for_each_component_impl(std::forward<F>(callback), *this, p);
		}

		template <class F, class PoolProvider>
		void for_each_component(F&& callback, PoolProvider& p) const {
			for_each_component_impl(std::forward<F>(callback), *this, p);
		}

		template <class F, class PoolProvider>
		void for_each_dynamic_component(F&& callback, PoolProvider& p) {
			for_each_dynamic_component_impl(std::forward<F>(callback), *this, p);
		}

		template <class F, class PoolProvider>
		void for_each_dynamic_component(F&& callback, PoolProvider& p) const {
			for_each_dynamic_component_impl(std::forward<F>(callback), *this, p);
		}

		template <class... excluded_components, class PoolProvider>
		void clone_components_except(
			const component_aggregate& from,
			PoolProvider& p
		) {
			from.for_each_component(
				[&](const auto& c){
					using component = std::decay_t<decltype(c)>;

					if constexpr(!is_one_of_v<component, excluded_components...>) {
						add(c, p);
					}
				}, 
				p
			);
		}
	};
}
