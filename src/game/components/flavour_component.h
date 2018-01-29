#pragma once
#include "game/transcendental/component_synchronizer.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/entity_flavour_id.h"

namespace augs {
	struct introspection_access;
}

namespace components {
	struct flavour {
		static constexpr bool is_always_present = true;
		static constexpr bool is_synchronized = true;

		// GEN INTROSPECTOR struct components::flavour
		entity_flavour_id flavour_id = 0u;
		// END GEN INTROSPECTOR
	};
}

template <bool is_const>
class basic_flavour_synchronizer : public component_synchronizer_base<is_const, components::flavour> {
	friend class flavour_id_cache;
protected:
	using base = component_synchronizer_base<is_const, components::flavour>;
	using base::handle;
public:
	using base::get_raw_component;
	using base::component_synchronizer_base;
	
	const entity_flavour& get_flavour() const;
	const entity_name_type& get_name() const;
	entity_flavour_id get_flavour_id() const;
};

template<>
class component_synchronizer<false, components::flavour> : public basic_flavour_synchronizer<false> {
	friend class flavour_id_cache;

public:
	using basic_flavour_synchronizer<false>::basic_flavour_synchronizer;
};

template<>
class component_synchronizer<true, components::flavour> : public basic_flavour_synchronizer<true> {
public:
	using basic_flavour_synchronizer<true>::basic_flavour_synchronizer;
};

entity_id get_first_named_ancestor(const const_entity_handle);
