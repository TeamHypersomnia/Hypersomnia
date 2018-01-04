#pragma once
#include <tuple>

#include "augs/callback_result.h"
#include "augs/misc/scope_guard.h"
#include "game/transcendental/cosmic_types.h"
#include "game/transcendental/entity_type_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/messages/will_soon_be_deleted.h"
#include "game/messages/queue_destruction.h"

class cosmos;

/*
	The purpose of this class is to centralize all functions 
	that can arbitrarily alter the solvable state inside the cosmos,
   	e.g. change fields of synchronized components and then refresh them accordingly.
*/

class cosmic {
	static void infer_caches_for(const entity_handle h);
	static void destroy_caches_of(const entity_handle h);

	static void infer_all_entities(cosmos& cosm);

	static void reinfer_solvable(cosmos&);
	
public:
	static entity_handle create_entity(cosmos&, entity_type_id);

	static entity_handle create_entity_with_specific_guid(
		cosmos&,
		const entity_guid specific_guid
	);

	static entity_handle clone_entity(const entity_handle);
	static void delete_entity(const entity_handle);

	static void reserve_storage_for_entities(cosmos&, const cosmic_pool_size_type s);
	static void increment_step(cosmos&);

	static void reinfer_all_entities(cosmos&);
	static void reinfer_caches_of(const entity_handle h);

	/* 
		Usually, type of an entity stays the same until its death.
		This function allows to change type of an already existing entity without changing its identity,
		so that existing entity_ids pointing to that entity are not broken.
	*/

	template <class entity_handle, class F>
	static void change_type(
		entity_handle handle,
		const entity_type_id new_id,
		F before_inference
	) {
		destroy_caches_of(handle);
		handle.get({}).template get<components::type>(handle.get_cosmos().get_solvable({})).type_id = new_id;

		/* TODO: Add initial components */

		before_inference();
		infer_caches_for(handle);
	}

	template <class C, class F>
	static void change_solvable_significant(C& cosm, F&& callback) {
		auto status = changer_callback_result::INVALID;

		auto refresh_when_done = augs::make_scope_guard([&]() {
			if (status != changer_callback_result::DONT_REFRESH) {
				reinfer_solvable(cosm);
			}
		});

		status = callback(cosm.get_solvable({}).significant);
	}

	template <class cache_type, class handle_type>
	static void reinfer_cache(const handle_type handle) {
		auto& cosm = handle.get_cosmos();
		auto& sys = std::get<cache_type&>(cosm.get_solvable_inferred({}).tie());

		sys.destroy_cache_of(handle);

		if (handle.is_inferred_state_activated()) {
			sys.infer_cache_for(handle);
		}
	}
};

/* Helper functions */

void make_deletion_queue(const const_entity_handle, deletion_queue& q);
void make_deletion_queue(const destruction_queue&, deletion_queue&, const cosmos& cosm);

deletion_queue make_deletion_queue(const const_entity_handle);
deletion_queue make_deletion_queue(const destruction_queue&, const cosmos& cosm);

void reverse_perform_deletions(const deletion_queue&, cosmos& cosm);
void delete_entity_with_children(const entity_handle);

