#include "game/transcendental/cosmic_functions.h"
#include "game/transcendental/cosmos.h"

void cosmic::infer_caches_for(const entity_handle h) {
	auto& cosm = h.get_cosmos();

	if (h.is_inferred_state_activated()) {
		auto constructor = [h](auto& sys) {
			sys.infer_cache_for(h);
		};

		cosm.get_solvable_inferred({}).for_each(constructor);
	}
}

void cosmic::destroy_caches_of(const entity_handle h) {
	auto& cosm = h.get_cosmos();

	auto destructor = [h](auto& sys) {
		sys.destroy_cache_of(h);
	};

	cosm.get_solvable_inferred({}).for_each(destructor);
}

void cosmic::infer_all_entities(cosmos& cosm) {
	for (const auto& ordered_pair : cosm.get_solvable().get_guid_to_id()) {
		infer_caches_for(cosm[ordered_pair.second]);
	}
}

void cosmic::reserve_storage_for_entities(cosmos& cosm, const cosmic_pool_size_type s) {
	cosm.get_solvable({}).reserve_storage_for_entities(s);
}

void cosmic::increment_step(cosmos& cosm) {
	cosm.get_solvable({}).increment_step();
}

void cosmic::reinfer_all_entities(cosmos& cosm) {
	auto scope = measure_scope(cosm.profiler.reinferring_all_entities);

	cosm.get_solvable({}).destroy_all_caches();
	infer_all_entities(cosm);
}

void cosmic::reinfer_solvable(cosmos& cosm) {
	cosm.get_solvable({}).remap_guids();
	reinfer_all_entities(cosm);
}

void cosmic::reinfer_caches_of(const entity_handle h) {
	destroy_caches_of(h);
	infer_caches_for(h);
}

