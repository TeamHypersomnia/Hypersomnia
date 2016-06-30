#include "entity_handle.h"
#include "game/detail/inventory_slot_id.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/components/relations_component.h"
#include "game/components/substance_component.h"
#include "game/components/processing_component.h"

#include "game/cosmos.h"

template <bool C>
template <class = typename std::enable_if<!C>::type>
basic_entity_handle<C>::operator basic_entity_handle<true>() {
	return basic_entity_handle<true>(owner, raw_id);
}

template <bool C>
bool basic_entity_handle<C>::operator==(entity_id b) const {
	return raw_id == b;
}

template <bool C>
bool basic_entity_handle<C>::operator!=(entity_id b) const {
	return raw_id != b;
}

template <bool C>
basic_entity_handle<C>::operator entity_id() const {
	return raw_id;
}

template <bool C>
bool basic_entity_handle<C>::is_in(processing_subjects list) const {
	return get<components::processing>().is_in(list);
}

template <bool C>
template <class = typename std::enable_if<!C>::type>
void basic_entity_handle<C>::skip_processing_in(processing_subjects) const {
	(*this)->removed_from_processing_subjects |= (1 << unsigned long long(list));
}

template <bool C>
template <class = typename std::enable_if<!C>::type>
void basic_entity_handle<C>::unskip_processing_in(processing_subjects) const {
	(*this)->removed_from_processing_subjects &=~ (1 << unsigned long long(list));
}

template <bool C>
template<class = typename std::enable_if<!C>::type>
components::substance& basic_entity_handle<C>::add(const components::substance& c) const {
	return aggregate::add<components::substance>();
}

template <bool C>
template <class = typename std::enable_if<!C>::type>
components::substance& basic_entity_handle<C>::add() const {
	return add(components::substance());
}

template <bool C>
template<class = typename std::enable_if<!C>::type>
components::processing& basic_entity_handle<C>::add(const components::processing& c) const {
	return aggregate::add(c);
}

template <bool C>
template <class = typename std::enable_if<!C>::type>
components::processing& basic_entity_handle<C>::add() const {
	return add(get_cosmos().stateful_systems.get<processing_lists_system>().get_standard_processing());
}


template <bool C>
template <class = typename std::enable_if<!C>::type>
void basic_entity_handle<C>::default_construct() {
	add<components::processing>();
	add<components::substance>();
}

// explicit instantiation
template class basic_entity_handle <false>;
template class basic_entity_handle <true>;