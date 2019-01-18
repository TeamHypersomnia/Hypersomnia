#pragma once
#include "view/audiovisual_state/systems/wandering_pixels_system.h"
#include "game/components/sprite_component.h"
#include "game/enums/render_layer.h"
#include "game/assets/animation_math.h"
#include "augs/drawing/sprite.hpp"

template <class E, class M>
void draw_wandering_pixels_as_sprites(
	const wandering_pixels_system& sys,
	const E subject_handle,
	const M& manager,
	invariants::sprite::drawing_input basic_input
) {
	subject_handle.template dispatch_on_having_all<invariants::wandering_pixels>([&](const auto subject) {
		const auto& wandering_def = subject.template get<invariants::wandering_pixels>();

		if (const auto cache = sys.find_cache(subject)) {
			for (const auto& p : cache->particles) {
				basic_input.renderable_transform = p.pos;

				{
					const auto& wandering = subject.template get<components::wandering_pixels>();
					basic_input.colorize = wandering.colorize;
				}

				const auto& cosm = subject_handle.get_cosmos();
				const auto& logicals = cosm.get_logical_assets();

				if (const auto displayed_animation = logicals.find(wandering_def.animation_id)) {
					const auto animation_time_ms = p.current_lifetime_ms;
					const auto image_id = ::calc_current_frame_looped(*displayed_animation, animation_time_ms).image_id;

					invariants::sprite animated;
					animated.image_id = image_id;
					animated.size = manager.at(image_id).get_original_size();

					augs::draw(animated, manager, basic_input);
				}
			}
		}
	});
}

