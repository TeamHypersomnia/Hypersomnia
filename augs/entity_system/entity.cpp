#pragma once
#include "entity.h"
#include "world.h"
#include "component_bitset_matcher.h"
#include "game/all_component_includes.h"

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

template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
remove_components(std::tuple<Tp...>& t, entity& e)
{ }

template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), void>::type
	remove_components(std::tuple<Tp...>& t, entity& e)
{
	typedef decltype(std::get<I>(t).second) comptypeptr;
	typedef std::remove_pointer<comptypeptr>::type component_type;

	if (std::get<I>(t).first.alive())
		e.remove<component_type>();

	remove_components<I + 1, Tp...>(t, e);
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
		ensure(!p->is_definition_entity());
		p->parent = get_id();
		p->name_as_sub_entity = optional_name;
		p->name_as_sub_definition = sub_definition_name::INVALID;
		sub_entities.push_back(p);
	}

	sub_entity_name entity::get_name_as_sub_entity() {
		return name_as_sub_entity;
	}

	sub_definition_name entity::get_name_as_sub_definition() {
		return name_as_sub_definition;
	}
	
	bool entity::is_definition_entity() {
		return born_as_definition_entity;
	}

	void entity::map_sub_entity(sub_entity_name n, augs::entity_id p) {
		ensure(p->parent.dead());
		ensure(!p->is_definition_entity());
		p->parent = get_id();
		p->name_as_sub_entity = n;
		p->name_as_sub_definition = sub_definition_name::INVALID;
		sub_entities_by_name[n] = p;
	}
	
	void entity::map_sub_definition(sub_definition_name n, augs::entity_id p) {
		ensure(p->parent.dead());
		ensure(p->is_definition_entity());
		p->parent = get_id();
		p->name_as_sub_definition = n;
		p->name_as_sub_entity = sub_entity_name::INVALID;
		sub_definitions[n] = p;
	}

	void entity::for_each_sub_entity(std::function<void(augs::entity_id)> f) {
		f(get_id());

		for (auto& e : sub_entities)
			e->for_each_sub_entity(f);

		for (auto& e : sub_entities_by_name)
			e.second->for_each_sub_entity(f);
	}

	void entity::for_each_sub_definition(std::function<void(augs::entity_id)> f) {
		for (auto& e : sub_definitions) {
			f(e.second);
			e.second->for_each_sub_definition(f);
		}
	}

	void entity::clone(augs::entity_id b) {
		ensure(b.alive());
		debug_name = b->debug_name;

		clone_components(b->type_to_component, *this);

		associated_entities_by_name = b->associated_entities_by_name;

		for (auto& s : b->sub_entities)
			add_sub_entity(owner_world.clone_entity(s));

		for (auto& s : b->sub_entities_by_name)
			map_sub_entity(s.first, owner_world.clone_entity(s.second));

		for (auto& s : b->sub_definitions)
			map_sub_definition(s.first, owner_world.clone_entity(s.second));

		remove<components::fixtures>();
		remove<components::physics>();

		if(find<components::item>())
			get<components::item>().current_slot.unset();

		// the same should be done with other sensitive pointers
	}

	component_bitset_matcher entity::get_component_signature() {
		return signature;
	}

	entity_id entity::get_id() {
		return self_id;// owner_world.get_id_from_raw_pointer(this);
	}

	void entity::clear() {
		remove_components(type_to_component, *this);
	}

	void entity::add_to_compatible_systems(size_t component_type_hash) {
		component_bitset_matcher old_signature(get_component_signature());
		signature.add(owner_world.component_library.get_index(component_type_hash));

		if (born_as_definition_entity)
			return;

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

		if (born_as_definition_entity)
			return true;

		entity_id this_id = get_id();

		for (auto sys : owner_world.get_all_systems())
			/* if a processing_system does not match with the new signature and does with the old one */
			if (!sys->components_signature.matches(new_signature) && sys->components_signature.matches(old_signature))
				/* we should remove this entity from there */
				sys->remove(this_id);

		return true;
	}
}