#include "entity_handle.h"
#include "game/components/substance_component.h"
#include "game/components/processing_component.h"

#include "game/cosmos.h"
#include "game/detail/physics_scripts.h"

namespace augs {
	template <bool C>
	bool basic_handle<C, cosmos, put_all_components_into<component_aggregate>::type>::operator==(entity_id b) const {
		return raw_id == b;
	}
}

/*
template <bool C>
template <class = typename std::enable_if<!C>::type>
basic_entity_handle<C>::operator basic_entity_handle<true>() const {
	return basic_entity_handle<true>(owner, raw_id);
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
*/

template <bool C>
template <class = typename std::enable_if<!C>::type>
void basic_handle<C, cosmos, put_all_components_into<component_aggregate>::type>::add_standard_components() {
	if (has<components::render>() && !is_entity_physical(*this) && !has<components::dynamic_tree_node>())
		add(components::dynamic_tree_node::get_default(*this));

	if (has<components::physics>() && !has<components::special_physics>())
		add<components::special_physics>();

	add(components::processing::get_default(*this));
	add<components::substance>();
}

// explicit instantiation
template class basic_handle<true, cosmos, put_all_components_into<component_aggregate>::type>;
template class basic_handle<false, cosmos, put_all_components_into<component_aggregate>::type>;