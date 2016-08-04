#pragma once
#include <type_traits>

#include "game/detail/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/entity_system/component_aggregate.h"
#include "augs/entity_system/aggregate_mixins.h"
#include "augs/misc/pool_handle.h"
#include "game/transcendental/entity_id.h"

#include "game/detail/entity/inventory_getters.h"
#include "game/detail/entity/physics_getters.h"
#include "game/detail/entity/relations_helpers.h"

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
		public relations_helpers<is_const, basic_entity_handle<is_const>>
	{
		friend class relations_helpers<is_const, basic_entity_handle<is_const>>;
		template <bool, class> friend class basic_relations_helpers;

		typedef augs::component_allocators<is_const, basic_entity_handle<is_const>> allocator;
		typedef basic_handle_base<is_const, cosmos, put_all_components_into<component_aggregate>::type> base;

		friend class augs::component_allocators<is_const, basic_entity_handle<is_const>>;

		template <class T, typename = void>
		struct component_or_synchronizer_or_disabled {
			typedef maybe_const_ref_t<is_const, T> return_type;

			basic_entity_handle<is_const> h;

			bool has() const {
				return h.allocator::template has<T>();
			}

			return_type get() const {
				return h.allocator::template get<T>();
			}

			void add(const T& t) const {
				h.allocator::add(t);
				
				if (std::is_same<T, components::substance>()) {
					h.get_cosmos().complete_resubstantialization(h);
				}
			}

			void remove() const {
				h.allocator::template remove<T>();

				if (std::is_same<T, components::substance>()) {
					h.get_cosmos().complete_resubstantialization(h);
				}
			}
		};

		template <class T>
		struct component_or_synchronizer_or_disabled<T, std::enable_if_t<is_component_synchronized<T>::value && !is_component_disabled<T>::value>> {
			typedef component_synchronizer<is_const, T> return_type;

			basic_entity_handle<is_const> h;

			bool has() const {
				return h.allocator::template has<T>();
			}

			return_type get() const {
				return component_synchronizer<is_const, T>(h.allocator::template get<T>(), h);
			}

			void add(const T& t) const {
				h.allocator::add(t);
				h.get_cosmos().complete_resubstantialization(h);
			}

			void remove() const {
				h.allocator::template remove<T>();
				h.get_cosmos().complete_resubstantialization(h);
			}
		};

		template <class T>
		struct component_or_synchronizer_or_disabled<T, std::enable_if_t<is_component_disabled<T>::value>> {
			typedef maybe_const_ref_t<is_const, T> return_type;

			basic_entity_handle<is_const> h;

			bool has() const {
				return false;
			}

			return_type get() const {
				static thread_local T t;
				t = T();
				return t;
			}

			void add(const T& t) const {

			}

			void remove() const {

			}
		};

		template<class T>
		using component_or_synchronizer_t = typename component_or_synchronizer_or_disabled<T>::return_type;

		using base::get;
		using base::get_meta;

	public:
		using base::base;

		typename base::owner_reference get_cosmos() const {
			return this->owner;
		}

		bool operator==(entity_id id) const {
			return base::operator==(id);
		}

		bool operator!=(entity_id id) const {
			return base::operator!=(id);
		}

		template <bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
		operator const_entity_handle() const {
			return const_entity_handle(this->owner, this->raw_id);
		}

		using base::operator entity_id;

		template <class component>
		bool has() const {
			return component_or_synchronizer_or_disabled<component>({ *this }).has();
		}

		template<class component>
		decltype(auto) get() const {
			return component_or_synchronizer_or_disabled<component>({ *this }).get();
		}

		template<class component, bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
		decltype(auto) add(const component& c) const {
			component_or_synchronizer_or_disabled<component>({ *this }).add(c);
			return get<component>();
		}

		template<class component, bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
		decltype(auto) add(const component_synchronizer<is_const, component>& c) const {
			component_or_synchronizer_or_disabled<component>({ *this }).add(c.get_data());
			return get<component>();
		}

		template<class component>
		decltype(auto) find() const {
			static_assert(!is_component_synchronized<component>::value, "Cannot return a pointer to synchronized component!");
			return allocator::template find<component>();
		}

		template<class component, bool _is_const = is_const, typename = std::enable_if_t<!_is_const>>
		void remove() const {
			return component_or_synchronizer_or_disabled<component>({ *this }).remove();
		}

		template<bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
		void add_standard_components() const;

		template<bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
		void recalculate_basic_processing_categories() const;
	};
}

template <bool is_const>
std::vector<entity_id> to_id_vector(std::vector<basic_entity_handle<is_const>> vec) {
	return std::vector<entity_id>(vec.begin(), vec.end());
}
