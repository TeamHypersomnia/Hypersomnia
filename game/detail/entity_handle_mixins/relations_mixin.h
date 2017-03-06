#pragma once
#include <vector>
#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/transcendental/entity_id.h"

#include "game/enums/slot_function.h"
#include "game/enums/child_entity_name.h"

#include "game/build_settings.h"
#include "augs/build_settings/setting_empty_bases.h"

#include "augs/templates/maybe_const.h"

struct entity_relations;

template<bool is_const, class entity_handle_type>
class basic_relations_mixin {
protected:
	typedef basic_inventory_slot_handle<is_const> inventory_slot_handle_type;
	
	const components::physical_relations& get_physical_relations_component() const;

public:
	entity_handle_type get_parent() const;
	
	entity_handle_type get_owner_body() const;
	std::vector<entity_handle_type> get_fixture_entities() const;

#if COSMOS_TRACKS_GUIDS
	unsigned get_guid() const;
#endif

	maybe_const_ref_t<is_const, child_entity_id> get_id(const child_entity_name) const;

	inventory_slot_handle_type operator[](const slot_function) const;
	entity_handle_type operator[](const child_entity_name) const;

	template <class F>
	void for_each_sub_entity_recursive(F callback) const {
		const auto self = *static_cast<const entity_handle_type*>(this);

		{
			const auto& subs = get_sub_entities_component().other_sub_entities;

			for (const auto& s : subs) {
				const auto handle = self.get_cosmos()[s];

				if (handle.alive()) {
					if (callback(handle)) {
						handle.for_each_sub_entity_recursive(callback);
					}
				}
			}
		}

		{
			const auto& subs = get_sub_entities_component().sub_entities_by_name;

			for (const auto& s : subs) {
				const auto handle = self.get_cosmos()[s.second];

				if (handle.alive()) {
					if (callback(handle)) {
						handle.for_each_sub_entity_recursive(callback);
					}
				}
			}
		}
	}
};

template<bool, class>
class relations_mixin;

template<class entity_handle_type>
class EMPTY_BASES relations_mixin<false, entity_handle_type> : public basic_relations_mixin<false, entity_handle_type> {
protected:
	components::physical_relations& physical_relations_component() const;

public:
	void make_child_of(const entity_id) const;

	void set_owner_body(const entity_id) const;
	void make_cloned_sub_entities_recursive(const entity_id copied) const;

	void map_child_entity(const child_entity_name n, const entity_id p) const;
};

template<class entity_handle_type>
class EMPTY_BASES relations_mixin<true, entity_handle_type> : public basic_relations_mixin<true, entity_handle_type> {
public:
};
