#include "name_component.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

namespace components {
	std::wstring name::get_nickname() const {
		return { nickname.begin(), nickname.end() };
	}

	void name::set_nickname(const std::wstring& s) {
		nickname.assign(s.begin(), s.end());
	}
}

void name_entity(const entity_handle& id, const entity_name n) {
	components::name name;
	name.id = n;
	id.set(name);
}

void name_entity(const entity_handle& id, const entity_name n, const std::wstring& nick) {
	components::name name;
	name.id = n;
	name.custom_nickname = true;
	name.set_nickname(nick);
	id.set(name);
}

entity_id get_first_named_ancestor(const const_entity_handle& p) {
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