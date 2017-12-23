#pragma once
#include "game/transcendental/component_synchronizer.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/components/type_component_declaration.h"

namespace augs {
	struct introspection_access;
}

class entity_type;

namespace components {
	struct type {
		static constexpr bool is_fundamental = true;
		static constexpr bool is_synchronized = true;

		friend augs::introspection_access;
		// GEN INTROSPECTOR struct components::type
		entity_type_id type_id = 0u;
		// END GEN INTROSPECTOR
	};
}

template <bool is_const>
class basic_type_synchronizer : public component_synchronizer_base<is_const, components::type> {
	friend class name_cache;
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
	friend class name_cache;

public:
	using basic_type_synchronizer<false>::basic_type_synchronizer;
};

template<>
class component_synchronizer<true, components::type> : public basic_type_synchronizer<true> {
public:
	using basic_type_synchronizer<true>::basic_type_synchronizer;
};

entity_id get_first_named_ancestor(const const_entity_handle);
