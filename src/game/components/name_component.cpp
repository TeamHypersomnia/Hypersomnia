#include "name_component.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/systems_inferred/name_system.h"
#include "game/components/all_inferred_state_component.h"

namespace components {
	name::name() {
		set_value(L"Unnamed");
	}

	entity_name_type name::get_value() const {
		return { value.begin(), value.end() };
	}

	void name::set_value(const entity_name_type& s) {
		value.assign(s.begin(), s.end());
		hash = static_cast<unsigned>(std::hash<std::wstring>()(std::wstring(L"unnamed")));
	}

	bool name::operator==(const name& b) const {
		return trivial_compare(*this, b);
	}

	bool name::operator!=(const name& b) const {
		return !operator==(b);
	}
}

entity_id get_first_named_ancestor(const const_entity_handle p) {
	entity_id iterator = p;
	const auto& cosmos = p.get_cosmos();

	while (cosmos[iterator].alive()) {
		if (cosmos[iterator].has<components::name>()) {
			return iterator;
			break;
		}

		iterator = cosmos[iterator].get_parent();
	}

	return entity_id();
}

typedef components::name N;

template <bool C>
entity_name_type basic_name_synchronizer<C>::get_value() const {
	return get_data().get_value();
}

void component_synchronizer<false, N>::set_value(const entity_name_type& new_name) const {
	handle.get_cosmos().systems_inferred.get<name_system>().set_name(
		handle, 
		new_name
	);
}

template class basic_name_synchronizer<false>;
template class basic_name_synchronizer<true>;