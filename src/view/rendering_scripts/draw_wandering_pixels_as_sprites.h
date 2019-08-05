#pragma once
#include "view/audiovisual_state/systems/wandering_pixels_system.h"
#include "game/components/sprite_component.h"
#include "game/enums/render_layer.h"
#include "game/assets/animation_math.h"
#include "augs/drawing/sprite.hpp"

template <class E, class M>
void draw_wandering_pixels_as_sprites(
	augs::vertex_triangle_buffer& triangles,
	int offset,
	const wandering_pixels_system& sys,
	const E& subject,
	const M& manager
) {
	const auto& wandering_def = subject.template get<invariants::wandering_pixels>();

	offset *= 2;

	if (const auto cache = sys.find_cache(subject.get_id())) {
		const auto& particles = cache->particles;

		for (std::size_t i = 0; i < particles.size(); ++i) {
			const auto& p = particles[i];
			const auto& wandering = subject.template get<components::wandering_pixels>();

			const auto& cosm = subject.get_cosmos();
			const auto& logicals = cosm.get_logical_assets();

			if (const auto displayed_animation = logicals.find(wandering_def.animation_id)) {
				const auto animation_time_ms = p.current_lifetime_ms;
				const auto image_id = ::calc_current_frame_looped(*displayed_animation, animation_time_ms).image_id;

				auto& t1 = triangles[offset + i * 2];
				auto& t2 = triangles[offset + i * 2 + 1];

				augs::detail_write_sprite(
					t1,
					t2,
					manager.at(image_id),
					p.pos,
					0,
					wandering.colorize
				);
			}
		}
	}
}

