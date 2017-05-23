#pragma once
#include <xstddef>
#include "augs/zeroed_pod.h"
#include "game/transcendental/component_synchronizer.h"
#include "augs/templates/is_component_synchronized.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/components/name_component_declaration.h"

namespace augs {
	struct introspection_access;
}

namespace components {
	struct name : synchronizable_component {
	private:
		friend struct augs::introspection_access;
		// GEN INTROSPECTOR struct components::name
		unsigned hash;
		fixed_entity_name_type value;
		// END GEN INTROSPECTOR
	public:
		name();

		bool operator==(const name&) const;
		bool operator!=(const name&) const;

		entity_name_type get_value() const;
		void set_value(const entity_name_type&);

		auto get_hash() const {
			return hash;
		}
	};
}

namespace std {
	template <>
	struct hash<components::name> {
		size_t operator()(const components::name& k) const {
			return std::hash<unsigned>()(k.get_hash());
		}
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
