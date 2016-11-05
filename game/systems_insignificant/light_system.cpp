#include "light_system.h"
#include "game/transcendental/step.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

void light_system::construct(const const_entity_handle) {

}

void light_system::destruct(const const_entity_handle) {

}

void light_system::reserve_caches_for_entities(const size_t n) {
	per_entity_cache.resize(n);
}

void light_system::render_all_lights(viewing_step& step) {
	const auto& cosmos = step.cosm;

	for (auto it : cosmos.get(processing_subjects::WITH_LIGHT)) {

	}
}