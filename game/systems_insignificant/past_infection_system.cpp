#include "past_infection_system.h"
#include "game/transcendental/entity_handle.h"

void past_infection_system::construct(const const_entity_handle e) {

}

void past_infection_system::destruct(const const_entity_handle e) {

}

void past_infection_system::reserve_caches_for_entities(const size_t n) {
	per_entity_cache.resize(n);
}