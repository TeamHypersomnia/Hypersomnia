#include "relations_mixin.h"
#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/components/substance_component.h"
#include "game/components/guid_component.h"
#include "game/components/child_component.h"
#include "game/components/physical_relations_component.h"
#include "game/components/crosshair_component.h"
#include "augs/templates/container_templates.h"

#include "generated_introspectors.h"

template <class D>
void relations_mixin<false, D>::make_child_of(const entity_id parent_id) const {
	auto& self = *static_cast<const D*>(this);

	auto& ch = self += components::child();
	ch.parent = parent_id;
}

template <class D>
components::physical_relations& relations_mixin<false, D>::physical_relations_component() const {
	auto& self = *static_cast<const D*>(this);

	if (!self.has<components::physical_relations>())
		self.add(components::physical_relations());

	return self.get<components::physical_relations>();
}

template <class D>
void relations_mixin<false, D>::make_cloned_child_entities_recursive(const entity_id from_id) const {
	auto& self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();
	
	const const_entity_handle from = cosmos[from_id];

	for_each_component_type([&](auto dum) {
		typedef decltype(dum) component_type;
		
		if (self.has<component_type>()) {
			ensure(from.has<component_type>());

			augs::introspect_recursive<
				is_entity_id_type,
				exclude_non_child_id_types
			> (
				[&](auto, auto& cloned_into_id, const auto& cloned_from_id) {
					cloned_into_id = cosmos.clone_entity(cloned_from_id);
				},
				synchronizer_or_component(self.get<component_type>()),
				synchronizer_or_component(from.get<component_type>())
			);
		}
	});
}

template <class D>
void relations_mixin<false, D>::set_owner_body(const entity_id owner_id) const {
	auto& self = *static_cast<const D*>(this);

	auto& cosmos = self.get_cosmos();
	auto new_owner = cosmos[owner_id];
	auto this_id = self.get_id();

	auto former_owner = cosmos[self.get_physical_relations_component().owner_body];

	if (former_owner.alive()) {
		remove_element(former_owner.physical_relations_component().fixture_entities, this_id);
		cosmos.partial_resubstantiation<physics_system>(former_owner);
	}

	self.physical_relations_component().owner_body = new_owner;

	if (new_owner.alive()) {
		remove_element(new_owner.physical_relations_component().fixture_entities, this_id);
		new_owner.physical_relations_component().fixture_entities.push_back(this_id);
		cosmos.partial_resubstantiation<physics_system>(new_owner);
	}
	else {
		cosmos.partial_resubstantiation<physics_system>(self);
	}
}

template <class D>
void relations_mixin<false, D>::map_child_entity(
	const child_entity_name n, 
	const entity_id p
) const {
	get_id(n) = p;
}

template <bool C, class D>
maybe_const_ref_t<C, child_entity_id> typename basic_relations_mixin<C, D>::get_id(const child_entity_name n) const {
	const auto& self = *static_cast<const D*>(this);

	switch (n) {
	case child_entity_name::CROSSHAIR_RECOIL_BODY:
		return self.get<components::crosshair>().recoil_entity;

	case child_entity_name::CHARACTER_CROSSHAIR:
		return self.get<components::sentience>().character_crosshair;

	case child_entity_name::CATRIDGE_ROUND:
		return self.get<components::catridge>().round;

	case child_entity_name::CATRIDGE_SHELL:
		return self.get<components::catridge>().shell;

	default:
		LOG("Access abstraction for this child_entity_name is not implemented!");
		ensure(false);
		return get_id(child_entity_name::CROSSHAIR_RECOIL_BODY);
	}
}

template <bool C, class D>
typename basic_relations_mixin<C, D>::inventory_slot_handle_type basic_relations_mixin<C, D>::operator[](const slot_function func) const {
	auto& self = *static_cast<const D*>(this);
	return inventory_slot_handle_type(self.owner, inventory_slot_id(func, self.raw_id));
}

template <bool C, class D>
D basic_relations_mixin<C, D>::operator[](const child_entity_name child) const {
	auto& self = *static_cast<const D*>(this);
	return self.get_cosmos()[get_id(child)];
}

template <bool C, class D>
D basic_relations_mixin<C, D>::get_owner_body() const {
	auto& self = *static_cast<const D*>(this);
	return self.get_cosmos()[get_physical_relations_component().owner_body];
}

template <bool C, class D>
std::vector<D> basic_relations_mixin<C, D>::get_fixture_entities() const {
	auto& self = *static_cast<const D*>(this);
	return self.get_cosmos()[get_physical_relations_component().fixture_entities];
}

#if COSMOS_TRACKS_GUIDS
template <bool C, class D>
unsigned basic_relations_mixin<C, D>::get_guid() const {
	auto& self = *static_cast<const D*>(this);
	return self.get<components::guid>().value;
}
#endif

template <bool C, class D>
D basic_relations_mixin<C, D>::get_parent() const {
	auto& self = *static_cast<const D*>(this);

	if (self.has<components::child>()) {
		return self.get_cosmos()[self.get<components::child>().parent];
	}
	else {
		return self.get_cosmos()[entity_id()];
	}
}

template <bool C, class D>
const components::physical_relations& basic_relations_mixin<C, D>::get_physical_relations_component() const {
	static thread_local const components::physical_relations original;
	
	const auto& self = *static_cast<const D*>(this);

	if (self.has<components::physical_relations>()) {
		return self.get<components::physical_relations>();
	}

	return original;
}

template class basic_relations_mixin<false, basic_entity_handle<false>>;
template class basic_relations_mixin<true, basic_entity_handle<true>>;
template class relations_mixin<false, basic_entity_handle<false>>;
template class relations_mixin<true, basic_entity_handle<true>>;
