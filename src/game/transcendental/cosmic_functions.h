#pragma once
#include <tuple>

#include "augs/callback_result.h"
#include "augs/misc/scope_guard.h"
#include "game/transcendental/cosmic_types.h"
#include "game/transcendental/entity_type_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/messages/will_soon_be_deleted.h"
#include "game/messages/queue_destruction.h"

class cosmic_delta;
class cosmos;

/*
	The purpose of this class is to centralize all functions 
	that can arbitrarily alter the solvable state inside the cosmos,
   	e.g. change fields of synchronized components and then refresh them accordingly.
*/

class cosmic {
	static void destroy_caches_of(const entity_handle h);

	static void infer_all_entities(cosmos& cosm);

	static void reinfer_solvable(cosmos&);
	
public:
	class specific_guid_creation_access {
		friend cosmic_delta;

		specific_guid_creation_access() {}
	};

	static entity_handle create_entity(cosmos&, entity_type_id);

	static entity_handle create_entity_with_specific_guid(
		specific_guid_creation_access,
		cosmos&,
		const entity_guid specific_guid
	);

	static entity_handle clone_entity(const entity_handle);
	static void delete_entity(const entity_handle);

	static void reserve_storage_for_entities(cosmos&, const cosmic_pool_size_type s);
	static void increment_step(cosmos&);

	static void reinfer_all_entities(cosmos&);
	static void infer_caches_for(const entity_handle h);

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
};

/* Helper functions */

void make_deletion_queue(const const_entity_handle, deletion_queue& q);
void make_deletion_queue(const destruction_queue&, deletion_queue&, const cosmos& cosm);

deletion_queue make_deletion_queue(const const_entity_handle);
deletion_queue make_deletion_queue(const destruction_queue&, const cosmos& cosm);

void reverse_perform_deletions(const deletion_queue&, cosmos& cosm);
void delete_entity_with_children(const entity_handle);

