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

template <class E>
class component_synchronizer<E, components::flavour> : public synchronizer_base<E, components::flavour> {
	friend class flavour_id_cache;
protected:
	using base = synchronizer_base<E, components::flavour>;
	using base::handle;
public:
	using base::get_raw_component;
	using base::synchronizer_base;

	const entity_flavour& get_flavour() const{
		return handle.get_cosmos().get_flavour(get_flavour_id());
	}

	const entity_name_type& get_name() const{
		return get_flavour().name;
	}

	entity_flavour_id get_flavour_id() const{
		return get_raw_component().flavour_id;
	}
};

entity_id get_first_named_ancestor(const const_entity_handle);
