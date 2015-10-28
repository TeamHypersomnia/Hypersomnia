#include "stdafx.h"
#include "camera_system.h"

#include <gl\GL.h>
#include "entity_system/entity.h"
#include "../components/physics_component.h"
#include "../messages/intent_message.h"
#include "../resources/render_info.h"
#include "entity_system/world.h"

#include <iostream>

void camera_system::consume_events(world& owner) {
	auto events = owner.get_message_queue<messages::intent_message>();

	for (auto it : events) {
		if (it.intent == messages::intent_message::intent_type::SWITCH_LOOK && it.state_flag) {
			auto& mode = it.subject->get<components::camera>().orbit_mode;
			if (mode == components::camera::LOOK)
				mode = components::camera::ANGLED;
			else mode = components::camera::LOOK;
		}
	}
}

void camera_system::process_entities(world& owner) {
	double delta = smooth_timer.extract<std::chrono::seconds>();

	/* we sort layers in reverse order to keep layer 0 as topmost and last layer on the bottom */
	std::sort(targets.begin(), targets.end(), [](entity_id a, entity_id b) {
		return a->get<components::camera>().layer > b->get<components::camera>().layer;
	});

	for (auto e : targets) {
		auto& camera = e->get<components::camera>();

		if (camera.enabled) {
			/* we obtain transform as a copy because we'll be now offsetting it by crosshair position */
			auto transform = e->get<components::transform>().current;
			transform.pos = vec2<int>(transform.pos);
			vec2<int> crosshair_offset;

			/* if we set player and crosshair entity targets */
			if (camera.player.alive() && camera.crosshair.alive()) {
				/* skip calculations if no orbit_mode is specified */
				if (camera.orbit_mode != camera.NONE) {
					/* shortcuts */
					vec2<int> crosshair_pos = camera.crosshair->get<components::transform>().current.pos;
					vec2<int> player_pos = camera.player->get<components::transform>().current.pos;
					vec2<> dir = (crosshair_pos - player_pos);

					dir.rotate(transform.rotation, vec2<>());

					if (camera.orbit_mode == camera.ANGLED) {
						vec2<> bound = camera.size / 2.f;
						/* save by copy */
						vec2<> normalized = dir.clamp(bound);
						crosshair_offset = normalized.normalize() * camera.angled_look_length;
					}

					if (camera.orbit_mode == camera.LOOK) {
						vec2<> bound = camera.max_look_expand + camera.size / 2.f;

						/* simple proportion in local frame of reference */
						vec2<> camera_offset = (dir.clamp(bound) / bound) * camera.max_look_expand;
						
						crosshair_offset = camera_offset.rotate(-transform.rotation, vec2<>());
					}
					
					/* rotate dir back */
					dir.rotate(-transform.rotation, vec2<>());
					/* update crosshair so it is snapped to visible area */
					camera.crosshair->get<components::transform>().current.pos = player_pos + dir;
				}
			}

			components::transform::state<> drawn_transform;
			augs::vec2<> drawn_size;

			if (camera.enable_smoothing) {
				/* variable time step camera smoothing by averaging last position with the current */
				double averaging_constant =
					pow(camera.smoothing_average_factor, camera.averages_per_sec * delta);
				
				if (camera.dont_smooth_once)
					averaging_constant = 0.0;

				//if ((transform.pos - camera.last_interpolant).length() < 2.0) camera.last_interpolant = transform.current.pos;
				//else

				vec2<int> target = transform.pos + crosshair_offset;
				vec2<int> smoothed_part = crosshair_offset;


				camera.last_interpolant.pos = camera.last_interpolant.pos * averaging_constant + vec2<double>(smoothed_part) * (1.0 - averaging_constant);
				camera.last_interpolant.rotation = camera.last_interpolant.rotation * averaging_constant + static_cast<double>(transform.rotation) * (1.0 - averaging_constant);
					
				auto interp = [](float& a, float& b, double averaging_constant){
					a = static_cast<float>(a * averaging_constant + b * (1.0 - averaging_constant));
				};

				interp(camera.last_ortho_interpolant.x, camera.size.x, averaging_constant);
				interp(camera.last_ortho_interpolant.y, camera.size.y, averaging_constant);

				/* save smoothing result */
				//if ((drawn_transform.pos - camera.last_interpolant.pos).length() > 5)
				drawn_transform.pos = target - smoothed_part + camera.last_interpolant.pos;
				drawn_transform.rotation = camera.last_interpolant.rotation;
				
				drawn_size = camera.last_ortho_interpolant;

				if (camera.crosshair_follows_interpolant) {
					camera.crosshair->get<components::transform>().current.pos -= transform.pos - camera.last_interpolant.pos;
				}
			}
			else {
				drawn_transform = transform;
				drawn_size = camera.size;

				drawn_transform.pos += crosshair_offset;
			}
			
			drawn_transform.pos = vec2<int>(drawn_transform.pos);
			drawn_size = vec2<int>(drawn_size);

			camera.dont_smooth_once = false;

			/* save the final smoothing results in previous transform state and component, we'll use them later in the rendering pass */
			e->get<components::transform>().previous = drawn_transform;
			camera.rendered_size = drawn_size;
		}
	}
}

void camera_system::process_rendering(world& owner) {
	render_system& raw_renderer = owner.get_system<render_system>();

	for (auto e : targets) {
		auto& camera = e->get<components::camera>();

		if (camera.enabled) {
			glViewport(camera.screen_rect.x, camera.screen_rect.y, camera.screen_rect.w, camera.screen_rect.h);
			
			auto drawn_transform = e->get<components::transform>().previous;

			resources::renderable::draw_input in;
			in.rotated_camera_aabb =
				rects::ltrb<float>::get_aabb_rotated(camera.rendered_size, drawn_transform.rotation) + drawn_transform.pos - camera.rendered_size / 2;
			in.camera_transform = drawn_transform;
			in.output = &raw_renderer;
			in.visible_area = camera.rendered_size;

			if (camera.drawing_callback) {
				try {
					/* arguments: subject, renderer, visible_area, target_transform, mask */
					luabind::call_function<void>(camera.drawing_callback, e, in, camera.mask);
				}
				catch (std::exception compilation_error) {
					std::cout << compilation_error.what() << '\n';
				}
			}
			else {
				raw_renderer.generate_triangles(in, camera.mask);
				raw_renderer.default_render(camera.rendered_size);

				if (raw_renderer.debug_drawing) {
					glDisable(GL_TEXTURE_2D);
					raw_renderer.draw_debug_info(camera.rendered_size, drawn_transform, nullptr);
					glEnable(GL_TEXTURE_2D);
				}
			}

		}
	}

	raw_renderer.cleanup();
}
