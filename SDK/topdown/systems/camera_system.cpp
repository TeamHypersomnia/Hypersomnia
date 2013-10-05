#include "stdafx.h"
#include "camera_system.h"

#include <gl\GL.h>
#include "entity_system/entity.h"
#include "../components/physics_component.h"
#include "../messages/intent_message.h"

camera_system::camera_system(render_system& raw_renderer) : raw_renderer(raw_renderer) {
	smooth_timer.reset();
}

void camera_system::process_entities(world& owner) {
	glClear(GL_COLOR_BUFFER_BIT);

	double delta = smooth_timer.extract<std::chrono::seconds>();

	/* we sort layers in reverse order to keep layer 0 as topmost and last layer on the bottom */
	std::sort(targets.begin(), targets.end(), [](entity* a, entity* b) {
		return a->get<components::camera>().layer > b->get<components::camera>().layer;
	});

	auto events = owner.get_message_queue<messages::intent_message>();

	for (auto it : events) {
		if (it.intent == messages::intent_message::intent_type::SWITCH_LOOK && it.state_flag) {
			auto& mode = it.subject->get<components::camera>().orbit_mode;
			if (mode == components::camera::LOOK)
				mode = components::camera::ANGLED;
			else mode = components::camera::LOOK;

		}
	}

	for (auto e : targets) {
		auto& camera = e->get<components::camera>();

		if (camera.enabled) {
			glLoadIdentity();
			glOrtho(camera.ortho.l, camera.ortho.r, camera.ortho.b, camera.ortho.t, 0, 1);
			glViewport(camera.screen_rect.x, camera.screen_rect.y, camera.screen_rect.w, camera.screen_rect.h);

			/* we obtain transform as a copy because we'll be now offsetting it by crosshair position */
			components::transform transform = e->get<components::transform>();
			vec2<> camera_screen = vec2<>(vec2<int>(camera.ortho.w(), camera.ortho.h()));

			/* if we set player and crosshair entity targets */
			if (camera.player && camera.crosshair) {
				/* skip calculations if no orbit_mode is specified */
				if (camera.orbit_mode != camera.NONE) {
					/* shortcuts */
					vec2<>& crosshair_pos = camera.crosshair->get<components::transform>().current.pos;
					vec2<> player_pos = camera.player->get<components::transform>().current.pos;
					vec2<> dir = (crosshair_pos - player_pos);

					if (camera.orbit_mode == camera.ANGLED) {
						vec2<> bound = camera_screen / 2.f;
						/* save copy */
						vec2<> normalized = dir.clamp(bound);
						transform.current.pos += normalized.normalize() * camera.angled_look_length;
					}

					if (camera.orbit_mode == camera.LOOK) {
						vec2<> bound = camera.max_look_expand + camera_screen / 2.f;
						/* simple proportion */
						transform.current.pos += (dir.clamp(bound) / bound) * camera.max_look_expand;
					}

					/* update crosshair so it is snapped to visible area */
					crosshair_pos = player_pos + dir;
				}
			}

			if (camera.enable_smoothing) {
				/* variable time step camera smoothing by averaging last position with the current */
				float averaging_constant = static_cast<float>(
					pow(camera.smoothing_average_factor, camera.averages_per_sec * delta)
					);

				//if ((transform.current.pos - camera.last_interpolant).length() < 2.0) camera.last_interpolant = transform.current.pos;
				//else
					camera.last_interpolant = camera.last_interpolant * averaging_constant + transform.current.pos * (1.0f - averaging_constant);
				/* save smoothing result */
				transform.current.pos = camera.last_interpolant;
			}

			raw_renderer.draw(camera.ortho, components::transform(transform.current.pos), camera.mask);
			raw_renderer.render();
		}
	}
	raw_renderer.output_window.swap_buffers();
}