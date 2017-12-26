#pragma once
#include "game/transcendental/component_synchronizer.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/entity_type_declaration.h"

namespace augs {
	struct introspection_access;
}


namespace components {
	struct type {
		static constexpr bool is_fundamental = true;
		static constexpr bool is_synchronized = true;

		// GEN INTROSPECTOR struct components::type
		entity_type_id type_id = 0u;
		// END GEN INTROSPECTOR
	};
}

template <bool is_const>
class basic_type_synchronizer : public component_synchronizer_base<is_const, components::type> {
	friend class type_id_cache;
protected:
	using base = component_synchronizer_base<is_const, components::type>;
	using base::handle;
public:
	using base::get_raw_component;
	using base::component_synchronizer_base;
	
	const entity_type& get_type() const;
	const entity_name_type& get_name() const;
	entity_type_id get_type_id() const;
};

template<>
class component_synchronizer<false, components::type> : public basic_type_synchronizer<false> {
	friend class type_id_cache;

public:
	using basic_type_synchronizer<false>::basic_type_synchronizer;

	void change_type_to(const entity_type_id id) const; 

	template <class F>
	void change_type_to(
		const entity_type_id id,
		F before_inference
	) const {
		auto& cosm = handle.get_cosmos();

		cosm.destroy_caches_of(handle);
		get_raw_component().type_id = id;

		/* TODO: Add initial components */

		before_inference();
		cosm.infer_caches_for(handle);
	}

};

template<>
class component_synchronizer<true, components::type> : public basic_type_synchronizer<true> {
public:
	using basic_type_synchronizer<true>::basic_type_synchronizer;
};

entity_id get_first_named_ancestor(const const_entity_handle);
