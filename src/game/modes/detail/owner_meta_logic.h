#pragma once

template <class A, class B>
void set_original_owner(const A& item_entity, const B& owner) {
	if (const auto item = item_entity.template find<components::item>()) {
		item.get_owner_meta().original_owner = owner;
	}
}

