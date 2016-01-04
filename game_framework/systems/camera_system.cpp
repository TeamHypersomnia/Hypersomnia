#include "math/vec2.h"
#include "camera_system.h"

#include <GL/OpenGL.h>
#include "entity_system/entity.h"
#include "../components/physics_component.h"
#include "../components/crosshair_component.h"
#include "../messages/intent_message.h"
#include "../shared/drawing_state.h"
#include "entity_system/world.h"

#include <iostream>

void camera_system::react_to_input_intents() {
	auto events = parent_world.get_message_queue<messages::intent_message>();

	for (auto it : events) {
		if (it.intent == messages::intent_message::intent_type::SWITCH_LOOK && it.pressed_flag) {
			auto& mode = it.subject->get<components::camera>().orbit_mode;
			if (mode == components::camera::LOOK)
				mode = components::camera::ANGLED;
			else mode = components::camera::LOOK;
		}
	}
}


void camera_system::crosshair_positioning_within_bounds() {

}

void camera_system::resolve_cameras_transforms_and_smoothing() {
	double delta = smooth_timer.extract<std::chrono::seconds>();

	/* we sort layers in reverse order to keep layer 0 as topmost and last layer on the bottom */
	std::sort(targets.begin(), targets.end(), [](entity_id a, entity_id b) {
		return a->get<components::camera>().layer > b->get<components::camera>().layer;
	});

	for (auto e : targets) {
		auto& camera = e->get<components::camera>();

		if (camera.enabled) {
			/* we obtain transform as a copy because we'll be now offsetting it by crosshair position */
			auto transform = e->get<components::transform>();
			transform.pos = vec2i(transform.pos);
			vec2i crosshair_offset;

			/* if we set player and crosshair entity targets */
			if (camera.player.alive() && camera.crosshair.alive()) {
				/* skip calculations if no orbit_mode is specified */
				if (camera.orbit_mode != camera.NONE) {
					/* shortcuts */
					vec2i player_pos = camera.player->get<components::transform>().pos;
					vec2i crosshair_pos = camera.crosshair->get<components::crosshair>().base_offset + player_pos;
					vec2 dir = (crosshair_pos - player_pos);

					dir.rotate(transform.rotation, vec2());

					if (camera.orbit_mode == camera.ANGLED) {
						vec2 bound = camera.size / 2.f;
						/* save by copy */
						vec2 normalized = dir.clamp(bound);
						crosshair_offset = normalized.normalize() * camera.angled_look_length;
					}

					if (camera.orbit_mode == camera.LOOK) {
						vec2 bound = camera.max_look_expand + camera.size / 2.f;

						/* simple proportion in local frame of reference */
						vec2 camera_offset = (dir.clamp(bound) / bound) * camera.max_look_expand;
						
						crosshair_offset = camera_offset.rotate(-transform.rotation, vec2());
					}
					
					/* rotate dir back */
					dir.rotate(-transform.rotation, vec2());
					/* update crosshair so it is snapped to visible area */
					camera.crosshair->get<components::transform>().pos = player_pos + dir;
					camera.crosshair->get<components::crosshair>().base_offset = dir;
				}
			}

			components::transform drawn_transform;
			vec2 drawn_size;

			drawn_transform = transform;
			drawn_size = camera.size;

			drawn_transform.pos += crosshair_offset;

			if (camera.enable_smoothing) {
				/* variable time step camera smoothing by averaging last position with the current */
				float averaging_constant =
					pow(camera.smoothing_average_factor, camera.averages_per_sec * delta);
				
				if (camera.dont_smooth_once)
					averaging_constant = 0.0f;

				//if ((transform.pos - camera.last_interpolant).length() < 2.0) camera.last_interpolant = transform.pos;
				//else

				vec2i target = transform.pos + crosshair_offset;
				vec2i smoothed_part = crosshair_offset;

				camera.last_interpolant.pos = camera.last_interpolant.pos * averaging_constant + vec2d(smoothed_part) * (1.0f - averaging_constant);
				camera.last_interpolant.rotation = camera.last_interpolant.rotation * averaging_constant + transform.rotation * (1.0f - averaging_constant);
					
				auto interp = [](float& a, float& b, float averaging_constant){
					a = a * averaging_constant + b * (1.0f - averaging_constant);
				};

				interp(camera.last_ortho_interpolant.x, camera.size.x, averaging_constant);
				interp(camera.last_ortho_interpolant.y, camera.size.y, averaging_constant);

				/* save smoothing result */
				//if ((drawn_transform.pos - camera.last_interpolant.pos).length() > 5)
				vec2 calculated_smoothed_pos = target - smoothed_part + camera.last_interpolant.pos;
				int calculated_smoothed_rotation = camera.last_interpolant.rotation;

				if (vec2i(calculated_smoothed_pos) == vec2i(drawn_transform.pos))
					camera.last_interpolant.pos = smoothed_part;
				if (int(calculated_smoothed_rotation) == int(drawn_transform.rotation))
					camera.last_interpolant.rotation = drawn_transform.rotation;
				/*
				if ((calculated_smoothed_pos - drawn_transform.pos).length_sq() < 1.f)
					camera.last_interpolant.pos = smoothed_part;
				if (std::abs(calculated_smoothed_rotation - drawn_transform.rotation) < 1.f)
					camera.last_interpolant.rotation = drawn_transform.rotation;
				*/

				drawn_transform.pos = calculated_smoothed_pos;
				drawn_transform.rotation = calculated_smoothed_rotation;

				//smoothing_player_pos

				drawn_size = camera.last_ortho_interpolant;

				if (camera.crosshair_follows_interpolant) {
					camera.crosshair->get<components::transform>().pos -= transform.pos - camera.last_interpolant.pos;
				}
			}
			
			drawn_transform.pos = vec2i(drawn_transform.pos);
			drawn_size = vec2i(drawn_size);

			camera.dont_smooth_once = false;

			camera.previously_drawn_at = drawn_transform;
			camera.rendered_size = drawn_size;
		}
	}
}

void camera_system::render_all_cameras() {
	auto& renderer = get_renderer();

	for (auto e : targets) {
		auto& camera = e->get<components::camera>();

		if (camera.enabled) {
			glViewport(camera.screen_rect.x, camera.screen_rect.y, camera.screen_rect.w, camera.screen_rect.h);
			
			auto drawn_transform = camera.previously_drawn_at;

			shared::drawing_state in;
			in.rotated_camera_aabb =
				rects::ltrb<float>::get_aabb_rotated(camera.rendered_size, drawn_transform.rotation) + drawn_transform.pos - camera.rendered_size / 2;
			in.camera_transform = drawn_transform;
			in.output = &renderer;
			in.visible_area = camera.rendered_size;

			if (camera.drawing_callback)
				camera.drawing_callback(e, in, camera.mask);
			else {
				parent_world.get_system<render_system>().generate_and_draw_all_layers(in, camera.mask);
				renderer.default_render(camera.rendered_size);

				if (renderer.debug_drawing) {
					glDisable(GL_TEXTURE_2D);
					renderer.draw_debug_info(camera.rendered_size, drawn_transform, assets::texture_id::BLANK, parent_world.get_system<render_system>().targets);
					glEnable(GL_TEXTURE_2D);
				}
			}
		}
	}

	renderer.clear_geometry();
}
