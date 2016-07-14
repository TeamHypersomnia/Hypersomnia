#pragma once
#include <type_traits>

#include "game/detail/inventory_slot_handle_declaration.h"
#include "game/entity_handle_declaration.h"

#include "entity_system/component_aggregate.h"
#include "entity_system/aggregate_mixins.h"
#include "misc/pool_handle.h"
#include "game/entity_id.h"

#include "game/detail/entity/inventory_getters.h"
#include "game/detail/entity/relations_component_helpers.h"
#include "game/detail/entity/physics_getters.h"

class cosmos;

template <bool, class>
class component_synchronizer;

namespace augs {
	template <bool is_const>
	class basic_handle<is_const, cosmos, put_all_components_into<component_aggregate>::type> :
		public basic_handle_base<is_const, cosmos, put_all_components_into<component_aggregate>::type>,

		private augs::component_allocators<is_const, basic_entity_handle<is_const>>,
		public augs::component_setters<is_const, basic_entity_handle<is_const>>,
		public inventory_getters<is_const, basic_entity_handle<is_const>>,
		public physics_getters<is_const, basic_entity_handle<is_const>>,
		public relations_component_helpers<is_const, basic_entity_handle<is_const>>
	{
		typedef augs::component_allocators<is_const, basic_entity_handle<is_const>> allocator;

		friend class augs::component_allocators<is_const, basic_entity_handle<is_const>>;

		template <class T, typename = void>
		struct component_or_synchronizer {
			typedef typename maybe_const_ref<is_const, T>::type return_type;

			basic_entity_handle<is_const> h;

			return_type get() const {
				return h.allocator::get<T>();
			}

			return_type add(const T& t) const {
				auto& ret = h.allocator::add(t);
				
				if (std::is_same<T, components::substance>()) {
					h.get_cosmos().complete_resubstantialization(h);
				}

				return ret;
			}

			void remove() const {
				h.allocator::remove<T>();

				if (std::is_same<T, components::substance>()) {
					h.get_cosmos().complete_resubstantialization(h);
				}
			}
		};

		template <class T>
		struct component_or_synchronizer<T, typename std::enable_if<is_component_synchronized<T>::value>::type> {
			typedef component_synchronizer<is_const, T> return_type;

			basic_entity_handle<is_const> h;

			return_type get() const {
				return component_synchronizer<is_const, T>(h.allocator::get<T>(), h);
			}

			return_type add(const T& t) const {
				ensure(!h.has<T>());
				auto added = component_synchronizer<is_const, T>(h.allocator::add(t), h);
				h.get_cosmos().complete_resubstantialization(h);
				return added;
			}

			void remove() const {
				ensure(h.has<T>());
				h.allocator::remove<T>();
				h.get_cosmos().complete_resubstantialization(h);
			}
		};
		

		using basic_handle_base::get;

	public:
		using basic_handle_base::basic_handle_base;

		typename basic_handle_base::owner_reference get_cosmos() const {
			return owner;
		}

		bool operator==(entity_id id) const {
			return basic_handle_base::operator==(id);
		}

		bool operator!=(entity_id id) const {
			return basic_handle_base::operator!=(id);
		}

		template <class = typename std::enable_if<!is_const>::type>
		operator basic_entity_handle<true>() const;
		operator entity_id() const;

		template <class component>
		bool has() const {
			return allocator::has<component>();
		}

		template<class component>
		decltype(auto) get() const {
			return component_or_synchronizer<component>({ *this }).get();
		}

		template<class component>
		typename std::enable_if<!is_const, typename component_or_synchronizer<component>::return_type>::type add(const component& c = component()) const {
			return component_or_synchronizer<component>({ *this }).add(c);
		}

		template<class component>
		typename std::enable_if<!is_const, typename component_or_synchronizer<component>::return_type>::type add(const component_or_synchronizer<component>& c = component()) const {
			return component_or_synchronizer<component>({ *this }).add(c.get_data());
		}

		template<class component>
		decltype(auto) find() const {
			static_assert(!is_component_synchronized<component>::value, "Cannot return a pointer to synchronized component!");
			return allocator::find<component>();
		}

		template<class component, typename = typename std::enable_if<!is_const>::type>
		void remove() const {
			return component_or_synchronizer<component>({ *this }).remove();
		}

		template<class = typename std::enable_if<!is_const>::type>
		void add_standard_components();
	};
}

template <bool is_const>
std::vector<entity_id> to_id_vector(std::vector<basic_entity_handle<is_const>> vec) {
	return std::vector<entity_id>(vec.begin(), vec.end());
}