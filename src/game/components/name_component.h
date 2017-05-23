#pragma once
#include "augs/zeroed_pod.h"
#include "game/transcendental/component_synchronizer.h"
#include "augs/templates/is_component_synchronized.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/components/name_component_declaration.h"

namespace components {
	struct name : synchronizable_component {
		// GEN INTROSPECTOR struct components::name
		fixed_entity_name_type value = entity_name_type(L"unnamed");
		// END GEN INTROSPECTOR

		entity_name_type get_value() const;
		void set_value(const entity_name_type&);
	};
}

template <bool is_const>
class basic_name_synchronizer : public component_synchronizer_base<is_const, components::name> {
	friend class name_system;
public:
	using component_synchronizer_base<is_const, components::name>::component_synchronizer_base;
	
	entity_name_type get_value() const;
};

template<>
class component_synchronizer<false, components::name> : public basic_name_synchronizer<false> {
	friend class name_system;

public:
	using basic_name_synchronizer<false>::basic_name_synchronizer;

	void set_value(const entity_name_type&) const;
};

template<>
class component_synchronizer<true, components::name> : public basic_name_synchronizer<true> {
public:
	using basic_name_synchronizer<true>::basic_name_synchronizer;
};

entity_id get_first_named_ancestor(const const_entity_handle);
