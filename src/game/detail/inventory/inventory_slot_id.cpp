#include "augs/ensure.h"
#include "augs/ensure_rel.h"
#include "inventory_slot_id.h"

template <class T>
bool basic_inventory_slot_id<T>::is_valid() const {
	if (type == slot_function::INVALID) {
		ensure(!container_entity.is_set());
	}
	else {
		ensure(container_entity.is_set());
	}

	if (container_entity.is_set()) {
		ensure(type != slot_function::INVALID);
	}
	else {
		ensure_eq(type, slot_function::INVALID);
	}

	return true;
}

template <class T>
basic_inventory_slot_id<T>::basic_inventory_slot_id(
	const slot_function f, 
	const T id
) : 
	type(f), 
	container_entity(id) 
{
	ensure(is_valid());
}

template <class T>
bool basic_inventory_slot_id<T>::operator==(const basic_inventory_slot_id b) const {
	return type == b.type && container_entity == b.container_entity;
}

template <class T>
bool basic_inventory_slot_id<T>::operator!=(const basic_inventory_slot_id b) const {
	return !operator==(b);
}

template <class T>
void basic_inventory_slot_id<T>::unset() {
	*this = basic_inventory_slot_id();
}
template <class T>
bool basic_inventory_slot_id<T>::is_set() const {
	return type != slot_function::INVALID && container_entity.is_set();
}

template struct basic_inventory_slot_id<entity_id>;
