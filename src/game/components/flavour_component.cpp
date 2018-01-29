#include "augs/build_settings/setting_debug_track_entity_name.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/flavour_component.h"

#include "game/inferred_caches/flavour_id_cache.h"

entity_id get_first_named_ancestor(const const_entity_handle p) {
	entity_id iterator = p;
	const auto& cosmos = p.get_cosmos();

	while (cosmos[iterator].alive()) {
		if (cosmos[iterator].get<components::flavour>().get_flavour_id() != 0) {
			return iterator;
		}

		iterator = cosmos[iterator].get_parent();
	}

	return entity_id();
}

typedef components::flavour N;

template <bool C>
const entity_flavour& basic_flavour_synchronizer<C>::get_flavour() const {
	return handle.get_cosmos().get_flavour(get_flavour_id());
}

template <bool C>
entity_flavour_id basic_flavour_synchronizer<C>::get_flavour_id() const {
	return get_raw_component().flavour_id;
}

template <bool C>
const entity_name_type& basic_flavour_synchronizer<C>::get_name() const {
	return get_flavour().name;
}

template class basic_flavour_synchronizer<false>;
template class basic_flavour_synchronizer<true>;