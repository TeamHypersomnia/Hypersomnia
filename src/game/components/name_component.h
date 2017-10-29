#pragma once
#include "game/transcendental/component_synchronizer.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/components/name_component_declaration.h"

namespace augs {
	struct introspection_access;
}

class entity_name_meta;

namespace components {
	struct name {
		static constexpr bool is_fundamental = true;
		static constexpr bool is_synchronized = true;

		friend augs::introspection_access;
		// GEN INTROSPECTOR struct components::name
		entity_name_id name_id = 0u;
		// END GEN INTROSPECTOR
	};
}

template <bool is_const>
class basic_name_synchronizer : public component_synchronizer_base<is_const, components::name> {
	friend class name_system;
protected:
	using base = component_synchronizer_base<is_const, components::name>;
	using base::handle;
public:
	using base::get_raw_component;
	using base::component_synchronizer_base;
	
	maybe_const_ref_t<is_const, entity_name_meta> get_meta() const;

	const entity_name_type& get_name() const;
	entity_name_id get_name_id() const;
};

template<>
class component_synchronizer<false, components::name> : public basic_name_synchronizer<false> {
	friend class name_system;

public:
	using basic_name_synchronizer<false>::basic_name_synchronizer;

	void set_name(const entity_name_type&) const;
	void set_name_id(const entity_name_id) const;
};

template<>
class component_synchronizer<true, components::name> : public basic_name_synchronizer<true> {
public:
	using basic_name_synchronizer<true>::basic_name_synchronizer;
};

entity_id get_first_named_ancestor(const const_entity_handle);
