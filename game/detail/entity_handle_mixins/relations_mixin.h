#pragma once
#include <vector>
#include "game/detail/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/transcendental/entity_id.h"

#include "game/enums/slot_function.h"
#include "game/enums/sub_entity_name.h"

#include "game/build_settings.h"

struct entity_relations;

template<bool is_const, class entity_handle_type>
class basic_relations_mixin {
protected:
	typedef basic_inventory_slot_handle<is_const> inventory_slot_handle_type;
	
	const components::child& get_child_component() const;
	const components::sub_entities& get_sub_entities_component() const;
	const components::physical_relations& get_physical_relations_component() const;

public:
	entity_handle_type get_parent() const;
	
	entity_handle_type get_owner_body() const;
	std::vector<entity_handle_type> get_fixture_entities() const;

#if COSMOS_TRACKS_GUIDS
	unsigned get_guid() const;
#endif

	inventory_slot_handle_type operator[](slot_function) const;
	entity_handle_type operator[](sub_entity_name) const;

	sub_entity_name get_name_as_sub_entity() const;
};

template<bool, class>
class relations_mixin;

template<class entity_handle_type>
class relations_mixin<false, entity_handle_type> : public basic_relations_mixin<false, entity_handle_type> {
protected:
	components::child& child_component() const;
	components::sub_entities& sub_entities_component() const;
	components::physical_relations& physical_relations_component() const;

	void make_child(entity_id, sub_entity_name) const;
public:
	void set_owner_body(entity_id) const;
	void make_cloned_sub_entities_recursive(entity_id copied) const;

	void add_sub_entity(entity_id p, sub_entity_name optional_name = sub_entity_name::INVALID) const;
	void map_sub_entity(sub_entity_name n, entity_id p) const;

	template <class F>
	void for_each_sub_entity_recursive(F callback) const {
		auto& self = *static_cast<const entity_handle_type*>(this);

		{
			auto& subs = sub_entities_component().other_sub_entities;

			for (auto& s : subs) {
				auto handle = self.get_cosmos()[s];

				if (handle.alive()) {
					callback(handle);
					handle.for_each_sub_entity_recursive(callback);
				}
			}
		}

		{
			auto& subs = sub_entities_component().sub_entities_by_name;

			for (auto& s : subs) {
				auto handle = self.get_cosmos()[s.second];

				if (handle.alive()) {
					callback(handle);
					handle.for_each_sub_entity_recursive(callback);
				}
			}
		}
	}
};

template<class entity_handle_type>
class relations_mixin<true, entity_handle_type> : public basic_relations_mixin<true, entity_handle_type> {
public:

	template <class F>
	void for_each_sub_entity_recursive(F callback) const {
		auto& self = *static_cast<const entity_handle_type*>(this);

		{
			auto& subs = get_sub_entities_component().other_sub_entities;

			for (const auto& s : subs) {
				auto handle = self.get_cosmos()[s];

				if (handle.alive()) {
					callback(handle);
					handle.for_each_sub_entity_recursive(callback);
				}
			}
		}

		{
			auto& subs = get_sub_entities_component().sub_entities_by_name;

			for (const auto& s : subs) {
				auto handle = self.get_cosmos()[s.second];

				if (handle.alive()) {
					callback(handle);
					handle.for_each_sub_entity_recursive(callback);
				}
			}
		}
	}
};
