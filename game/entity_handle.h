#pragma once
#include <type_traits>

#include "game/detail/inventory_slot_handle_declaration.h"
#include "game/entity_handle_declaration.h"

#include "entity_system/component_aggregate.h"
#include "entity_system/aggregate_mixins.h"
#include "misc/pool_handle.h"
#include "game/entity_id.h"

#include "game/detail/entity/inventory_getters.h"
#include "game/detail/entity/physics_getters.h"
#include "game/detail/entity/relations_component_helpers.h"

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
			typedef maybe_const_ref_t<is_const, T> return_type;

			basic_entity_handle<is_const> h;

			return_type get() const {
				return h.allocator::get<T>();
			}

			void add(const T& t) const {
				h.allocator::add(t);
				
				if (std::is_same<T, components::substance>()) {
					h.get_cosmos().complete_resubstantialization(h);
				}
			}

			void remove() const {
				h.allocator::remove<T>();

				if (std::is_same<T, components::substance>()) {
					h.get_cosmos().complete_resubstantialization(h);
				}
			}
		};

		template <class T>
		struct component_or_synchronizer<T, std::enable_if_t<is_component_synchronized<T>::value>> {
			typedef component_synchronizer<is_const, T> return_type;

			basic_entity_handle<is_const> h;

			return_type get() const {
				return component_synchronizer<is_const, T>(h.allocator::get<T>(), h);
			}

			void add(const T& t) const {
				ensure(!h.has<T>());
				h.allocator::add(t);
				h.get_cosmos().complete_resubstantialization(h);
			}

			void remove() const {
				ensure(h.has<T>());
				h.allocator::remove<T>();
				h.get_cosmos().complete_resubstantialization(h);
			}
		};
		
		template<class T>
		using component_or_synchronizer_t = typename component_or_synchronizer<T>::return_type;

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

		template <class = std::enable_if_t<!is_const>>
		
		operator const_entity_handle() const {
			return const_entity_handle(owner, raw_id);
		}

		using basic_handle_base::operator entity_id;

		template <class component>
		bool has() const {
			return allocator::has<component>();
		}

		template<class component>
		decltype(auto) get() const {
			ensure(has<component>());
			return component_or_synchronizer<component>({ *this }).get();
		}

		template<class component, class = std::enable_if_t<!is_const>>
		decltype(auto) add(const component& c) const {
			ensure(!has<component>());
			component_or_synchronizer<component>({ *this }).add(c);
			return get<component>();
		}

		template<class component, class = std::enable_if_t<!is_const>>
		decltype(auto) add(const component_synchronizer<is_const, component>& c) const {
			ensure(!has<component>());
			component_or_synchronizer<component>({ *this }).add(c.get_data());
			return get<component>();
		}

		template<class component>
		decltype(auto) find() const {
			static_assert(!is_component_synchronized<component>::value, "Cannot return a pointer to synchronized component!");
			return allocator::find<component>();
		}

		template<class component, typename = std::enable_if_t<!is_const>>
		void remove() const {
			return component_or_synchronizer<component>({ *this }).remove();
		}

		template<class = std::enable_if_t<!is_const>>
		void add_standard_components();
	};
}

template <bool is_const>
std::vector<entity_id> to_id_vector(std::vector<basic_entity_handle<is_const>> vec) {
	return std::vector<entity_id>(vec.begin(), vec.end());
}