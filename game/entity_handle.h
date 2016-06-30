#pragma once
#include <functional>
#include <type_traits>

#include "game/enums/slot_function.h"
#include "game/enums/associated_entity_name.h"
#include "game/enums/sub_entity_name.h"
#include "game/enums/sub_entity_name.h"

#include "game/enums/sub_entity_name.h"
#include "game/enums/processing_subjects.h"

#include "game/detail/inventory_slot_handle_declaration.h"
#include "game/entity_handle_declaration.h"

#include "entity_system/aggregate_handle.h"
#include "game/entity_id.h"

#include "game/components/processing_component.h"

namespace components {
	struct relations;
}

class cosmos;

template <bool is_const>
using basic_entity_handle_base = augs::basic_aggregate_handle<is_const, cosmos, put_all_components_into<std::tuple>::type>;

template <bool is_const>
class basic_entity_handle : private basic_entity_handle_base<is_const>, public aggregate_setters<is_const, basic_entity_handle<is_const>> {
	typedef basic_entity_handle_base<is_const> aggregate;
	typedef typename maybe_const_ref<is_const, components::relations>::type relations_type;
	typedef typename basic_inventory_slot_handle<is_const> inventory_slot_handle_type;

	relations_type relations() const;

	template <class T, typename=void>
	struct component_or_synchronizer {
		basic_entity_handle h;

		decltype(auto) get() const {
			return h.aggregate::get<T>();
		}

		decltype(auto) add(const T& t) const {
			return h.aggregate::add(t);
		}

		decltype(auto) remove() const {
			return h.aggregate::remove<T>();
		}
	};

	template <class T>
	struct component_or_synchronizer<T, typename std::enable_if<is_component_synchronized<T>::value>::type> {
		basic_entity_handle h;

		auto get() const {
			return component_synchronizer<is_const, T>(h.aggregate::get<T>(), h);
		}

		auto add(const T& t) const {
			ensure(!h.has<T>());

			return component_synchronizer<is_const, T>(h.aggregate::add(t), h);
		}

		void remove() const {
			ensure(h.has<T>());
			component_synchronizer<is_const, T> sync(h.aggregate::get<T>(), h);


			h.aggregate::remove<T>();
		}
	};
public:

	using aggregate::aggregate;
	
	basic_entity_handle make_handle(entity_id) const;

	auto& get_cosmos() const {
		return owner;
	}

	basic_entity_handle get_parent() const;

	inventory_slot_handle_type operator[](slot_function) const;
	basic_entity_handle operator[](sub_entity_name) const;
	basic_entity_handle operator[](associated_entity_name) const;

	void for_each_sub_entity_recursive(std::function<void(basic_entity_handle)>) const;

	bool has(sub_entity_name) const;
	bool has(associated_entity_name) const;
	bool has(slot_function) const;

	template <class = typename std::enable_if<!is_const>::type>
	void add_sub_entity(entity_id p, sub_entity_name optional_name = sub_entity_name::INVALID) const;

	template <class = typename std::enable_if<!is_const>::type>
	void map_sub_entity(sub_entity_name n, entity_id p) const;

	template <class = typename std::enable_if<!is_const>::type>
	void map_associated_entity(associated_entity_name n, entity_id p) const;


	template <class = typename std::enable_if<!is_const>::type>
	operator basic_entity_handle<true>();

	bool operator==(entity_id b) const;
	bool operator!=(entity_id b) const;

	operator entity_id() const;

	bool is_in(processing_subjects) const;

	template <class = typename std::enable_if<!is_const>::type>
	void skip_processing_in(processing_subjects) const;

	template <class = typename std::enable_if<!is_const>::type>
	void unskip_processing_in(processing_subjects) const;

	template<class = typename std::enable_if<!is_const>::type>
	components::substance& add(const components::substance& c) const;

	template<class = typename std::enable_if<!is_const>::type>
	components::substance& add() const;

	template<class = typename std::enable_if<!is_const>::type>
	components::processing& add(const components::processing& c) const;

	template<class = typename std::enable_if<!is_const>::type>
	components::processing& add() const;

	template <class component>
	bool has() const {
		return aggregate::has<component>();
	}

	template<class component>
	decltype(auto) get() const {
		return component_or_synchronizer<component>({ *this }).get();
	}

	template<class component>
	decltype(auto) add(const component& c) const {
		return component_or_synchronizer<component>({ *this }).add(c);
	}

	template<class component>
	decltype(auto) find() const {
		static_assert(!is_component_synchronized<component>::value, "Cannot return a pointer to synchronized component!");
		return aggregate::find<component>();
	}

	template<class component>
	void remove() const {
		return component_or_synchronizer<component>({ *this }).remove();
	}

	template<class = typename std::enable_if<!is_const>::type>
	void default_construct();
	
	basic_entity_handle get_owning_transfer_capability() const;

	inventory_slot_handle_type determine_hand_holstering_slot(basic_entity_handle searched_root_container) const;
	inventory_slot_handle_type determine_pickup_target_slot_in(basic_entity_handle searched_root_container) const;

	inventory_slot_handle_type first_free_hand() const;

	inventory_slot_handle_type map_primary_action_to_secondary_hand_if_primary_empty(int is_action_secondary) const;

	basic_entity_handle get_owner_friction_field() const;
	basic_entity_handle get_owner_body_entity() const;

	std::vector<basic_entity_handle> guns_wielded();
};

template <bool is_const>
std::vector<entity_id> to_id_vector(std::vector<basic_entity_handle<is_const>> vec) {
	return std::vector<entity_id>(vec.begin(), vec.end());
}