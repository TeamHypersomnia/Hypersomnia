#pragma once
#include <array>
#include <unordered_map>

#include "augs/misc/timing/delta.h"
#include "augs/misc/randomization.h"

#include "game/enums/render_layer.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"

#include "game/components/wandering_pixels_component.h"
#include "game/components/sprite_component.h"
#include "view/audiovisual_state/systems/audiovisual_cache_common.h"

struct visible_entities;

class interpolation_system;
struct particles_emission;

class wandering_pixels_system {
public:
	struct particle {
		vec2 pos;
		vec2 current_direction = vec2(1, 0);
		float current_velocity = 20.f;
		float direction_ms_left = 0.f;
		float current_lifetime_ms = 0.f;
	};

	struct cache {
		components::wandering_pixels recorded_component;

		std::vector<particle> particles;
		bool constructed = false;
	};

	double global_time_seconds = 0.0;

	audiovisual_cache_map<cache> per_entity_cache;

	cache& get_cache(const const_entity_handle);
	const cache& get_cache(const const_entity_handle) const;

	void advance_for(
		const visible_entities& subjects,
		const cosmos&,
		const augs::delta dt
	);

	void advance_for(
		const const_entity_handle subject,
		const augs::delta dt
	);

	template <class E, class M>
	void draw_wandering_pixels_as_sprites(
		const E subject_handle,
		const M& manager,
		invariants::sprite::drawing_input basic_input
	) const {
		subject_handle.template dispatch_on_having<invariants::wandering_pixels>([&](const auto subject){
			const auto& wandering_def = subject.template get<invariants::wandering_pixels>();
			const auto& cache = get_cache(subject);

			for (const auto& p : cache.particles) {
				basic_input.renderable_transform = p.pos;

				if (wandering_def.frames.size() > 0) {
					{
						const auto& wandering = subject.template get<components::wandering_pixels>();
						basic_input.colorize = wandering.colorize;
					}

					wandering_def.get_face_after(p.current_lifetime_ms).draw(manager, basic_input);
				}
			}
		});
	}

	void reserve_caches_for_entities(const size_t) const {}

	void clear();
	void clear_dead_entities(const cosmos&);
};