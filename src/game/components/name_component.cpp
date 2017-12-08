#include "augs/build_settings/setting_debug_track_entity_name.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/all_inferred_state_component.h"
#include "game/components/name_component.h"

#include "game/inferred_caches/name_cache.h"

entity_id get_first_named_ancestor(const const_entity_handle p) {
	entity_id iterator = p;
	const auto& cosmos = p.get_cosmos();

	while (cosmos[iterator].alive()) {
		if (cosmos[iterator].get<components::name>().get_name_id() != 0) {
			return iterator;
		}

		iterator = cosmos[iterator].get_parent();
	}

	return entity_id();
}

typedef components::name N;

template <bool C>
maybe_const_ref_t<C, entity_name_meta> basic_name_synchronizer<C>::get_meta() const {
	return handle.get_cosmos().get_common_state().name_metas.get_meta(get_name_id());
}

template <bool C>
entity_name_id basic_name_synchronizer<C>::get_name_id() const {
	return get_raw_component().name_id;
}

template <bool C>
const entity_name_type& basic_name_synchronizer<C>::get_name() const {
	const auto& cosmos = handle.get_cosmos();

	return cosmos.inferred.name.get_name(
		cosmos.get_common_state().name_metas,
		get_raw_component()
	);
}

void component_synchronizer<false, N>::set_name(const entity_name_type& full_name) const {
	auto& cosmos = handle.get_cosmos();

	cosmos.inferred.name.set_name(
		cosmos.get_common_state().name_metas,
		full_name,
		get_raw_component(),
		handle
	);

#if DEBUG_TRACK_ENTITY_NAME
	/* TODO: Come up with something so that we can check entity names while debugging */
#endif
}

void component_synchronizer<false, N>::set_name_id(const entity_name_id id) const {
	handle.get_cosmos().inferred.name.set_name_id(
		id,
		get_raw_component(),
		handle
	);

#if DEBUG_TRACK_ENTITY_NAME
	/* TODO: Come up with something so that we can check entity names while debugging */
#endif
}

template class basic_name_synchronizer<false>;
template class basic_name_synchronizer<true>;