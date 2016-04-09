#pragma once
#include "entity.h"
#include "world.h"
#include "component_bitset_matcher.h"
#include "game_framework/all_component_includes.h"

#include <tuple>
#include <utility> 

#include "ensure.h"

template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
clone_components (std::tuple<Tp...>& t, entity& e)
{ }

template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), void>::type
	clone_components(std::tuple<Tp...>& t, entity& e)
{
	typedef decltype(std::get<I>(t).second) comptypeptr;
	typedef std::remove_pointer<comptypeptr>::type component_type;
	
	if(std::get<I>(t).first.alive())
		e.add<component_type>(*(component_type*)(std::get<I>(t).first.ptr()));
	
	clone_components<I + 1, Tp...>(t, e);
}

namespace augs {
	entity::entity(world& owner_world) : owner_world(owner_world) {}

	entity::~entity() {
		clear();
	}

	augs::entity_id entity::get_parent() {
		return parent;
	}

	void entity::add_sub_entity(augs::entity_id p, sub_entity_name optional_name) {
		ensure(p->parent.dead());
		p->parent = get_id();
		p->name_as_subentity = optional_name;
		sub_entities.push_back(p);
	}

	sub_entity_name entity::get_name_as_subentity() {
		return name_as_subentity;
	}

	void entity::map_sub_entity(sub_entity_name n, augs::entity_id p) {
		ensure(p->parent.dead());
		p->parent = get_id();
		p->name_as_subentity = n;
		sub_entities_by_name[n] = p;
	}

	void entity::for_each_subentity(std::function<void(augs::entity_id)> f) {
		f(get_id());

		for (auto& e : sub_entities)
			e->for_each_subentity(f);

		for (auto& e : sub_entities_by_name)
			e.second->for_each_subentity(f);
	}

	void entity::clone(augs::entity_id b) {
		ensure(b.alive());
#if USE_POINTER_TUPLE
		debug_name = b->debug_name;
		
		clone_components(b->type_to_component, *this);

		associated_entities_by_name = b->associated_entities_by_name;

		for (auto& s : b->sub_entities)
			add_sub_entity(owner_world.clone_entity(s));

		for (auto& s : b->sub_entities_by_name)
			map_sub_entity(s.first, owner_world.clone_entity(s.second));

		remove<components::fixtures>();
		remove<components::physics>();

		if(find<components::item>())
			get<components::item>().current_slot.unset();

		// the same should be done with other sensitive pointers
#else
		ensure(0);
#endif
	}

	component_bitset_matcher entity::get_component_signature() {
#if USE_POINTER_TUPLE
		return signature;
#else
		std::vector<unsigned> indices;

		for (auto& raw : type_to_component.raw) {
			int index = owner_world.component_library.get_index(raw.key);
			indices.push_back(index);
		}

		return indices;
#endif
	}

	entity_id entity::get_id() {
		return self_id;// owner_world.get_id_from_raw_pointer(this);
	}

	void entity::clear() {
#if USE_POINTER_TUPLE
		remove_all(type_to_component);
#else
		auto ids_to_remove = type_to_component.raw;

		for (auto c : ids_to_remove)
			remove(c.key);
#endif
	}

	void entity::add_to_compatible_systems(size_t component_type_hash) {
		component_bitset_matcher old_signature(get_component_signature());
		signature.add(owner_world.component_library.get_index(component_type_hash));

		entity_id this_id = get_id();
		for (auto sys : owner_world.get_all_systems())
		{
			bool matches_new = sys->components_signature.matches(signature);
			bool doesnt_match_old = !sys->components_signature.matches(old_signature);

			if (matches_new && doesnt_match_old)
				sys->add(this_id);
		}
	}

	bool entity::unplug_component_from_systems(size_t component_type_hash) {
		component_bitset_matcher old_signature(get_component_signature());

		component_bitset_matcher& new_signature = signature;
		signature.remove(owner_world.component_library.get_index(component_type_hash));

		bool is_already_removed = old_signature == new_signature;

		if (is_already_removed)
			return false;

		entity_id this_id = get_id();

		for (auto sys : owner_world.get_all_systems())
			/* if a processing_system does not match with the new signature and does with the old one */
			if (!sys->components_signature.matches(new_signature) && sys->components_signature.matches(old_signature))
				/* we should remove this entity from there */
				sys->remove(this_id);

		return true;
	}

	void entity::remove(size_t component_type_hash) {
#if !USE_POINTER_TUPLE
		remove_from_incompatible_systems(component_type_hash);

		auto* component_ptr = type_to_component.get(component_type_hash);

		/* delete component from the corresponding pool, use hash to identify the proper destructor */
		owner_world.get_components_by_hash(component_type_hash).free_with_destructor(*component_ptr, component_type_hash);

		/* delete component entry from entity's map */
		type_to_component.remove(component_type_hash);

#ifdef INCLUDE_COMPONENT_NAMES
		typestrs.remove(component_type_hash);
#endif
#else
		ensure(0);
#endif
	}
}