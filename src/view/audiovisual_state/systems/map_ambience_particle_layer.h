#pragma once
#include <type_traits>
#include "game/enums/particle_layer.h"
#include "game/cosmos/typed_entity_handle_declaration.h"

struct area_sensor;
struct particles_decoration;

template <class E>
constexpr bool emits_map_ambience_v =
	std::is_same_v<E, area_sensor>
	|| std::is_same_v<E, particles_decoration>
;

inline particle_layer map_ambience_layer_instead_of(const particle_layer layer) {
	if (layer == particle_layer::ILLUMINATING_PARTICLES) {
		return particle_layer::MAP_AMBIENCE_ILLUMINATING_PARTICLES;
	}

	return layer;
}

template <class H, class EmissionInstances>
void move_map_ambience_to_its_own_particle_pool(const H&, EmissionInstances& emission_instances) {
	using E = entity_type_of<H>;

	if constexpr(emits_map_ambience_v<E>) {
		for (auto& instance : emission_instances) {
			auto& layer = instance.source_emission.target_layer;
			layer = ::map_ambience_layer_instead_of(layer);
		}
	}
	else {
		(void)emission_instances;
	}
}
