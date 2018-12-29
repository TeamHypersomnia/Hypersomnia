#include "inventory_slot_id.h"

template <class T>
basic_inventory_slot_id<T>::basic_inventory_slot_id() : type(slot_function::INVALID) {

}

template <class T>
basic_inventory_slot_id<T>::basic_inventory_slot_id(
	const slot_function f, 
	const T id
) : 
	type(f), 
	container_entity(id) 
{}

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
	return *this != basic_inventory_slot_id();
}

template struct basic_inventory_slot_id<entity_id>;
