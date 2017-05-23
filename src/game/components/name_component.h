#pragma once
#include "augs/zeroed_pod.h"
#include "augs/misc/constant_size_vector.h"

#include "game/container_sizes.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

using entity_name_type = std::wstring;
using fixed_entity_name_type = augs::constant_size_wstring<ENTITY_NAME_LENGTH>;

static_assert(fixed_entity_name_type::array_size % 4 == 0, "Wrong entity name padding");

namespace components {
	struct name : synchronizable_component {
		// GEN INTROSPECTOR struct components::name
		fixed_entity_name_type value = entity_name_type("unnamed");
		// END GEN INTROSPECTOR

		entity_name_type get_value() const;
		void set_value(const entity_name_type&);
	};
}

template <bool is_const>
class basic_name_synchronizer : public component_synchronizer_base<is_const, components::name> {
protected:
public:
	using component_synchronizer_base<is_const, components::name>::component_synchronizer_base;
	
	entity_name_type get_value() const;
};

template<>
class component_synchronizer<false, components::name> : public basic_name_synchronizer<false> {
	void reinference() const;

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
