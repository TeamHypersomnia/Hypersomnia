#pragma once
#include "entity_id.h"
#include "full_entity_definition.h"
#include "game/entity_handle.h"

class definition_interface {
	full_entity_definition def;
	entity_handle id;

public:
	definition_interface(entity_handle);

	template<class component>
	bool is_set() const {
		return def ? def.is_set<component>() : id.has<component>();
	}

	template<class component>
	component& set(const component& c = component()) {
		return def ? def.set(c) : id.set(c);
	}

	template<class... components>
	void set(components... args) {
		return def ? def.set(args...) : id.set(args...);
	}

	template<class component>
	component& operator+=(const component& c) {
		return def ? def.set(c) : id.set(c);
	}

	template<class component>
	component& get() {
		return def ? def.get<component>() : id.get<component>();
	}

	template<class component>
	const component& get() const {
		return def ? def.get<component>() : id.get<component>();
	}

	template<class component>
	component* find() {
		return def ? def.find<component>() : id.find<component>();
	}

	template<class component>
	const component* find() const {
		return def ? def.find<component>() : id.find<component>();
	}
};