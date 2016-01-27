#include "math/vec2.h"
#include "camera_system.h"

#include "entity_system/entity.h"
#include "../components/physics_component.h"
#include "../components/crosshair_component.h"
#include "../components/position_copying_component.h"
#include "../components/crosshair_component.h"
#include "../components/physics_component.h"
#include "../messages/intent_message.h"
#include "../messages/camera_render_request_message.h"
#include "../shared/drawing_state.h"
#include "entity_system/world.h"

#include "augs/print.h"

void camera_system::react_to_input_intents() {
	auto events = parent_world.get_message_queue<messages::intent_message>();

	for (auto it : events) {
		if (it.subject->find<components::camera>() == nullptr)
			continue;

		if (it.intent == messages::intent_message::intent_type::SWITCH_LOOK && it.pressed_flag) {
			auto& mode = it.subject->get<components::camera>().orbit_mode;
			if (mode == components::camera::LOOK)
				mode = components::camera::ANGLED;
			else mode = components::camera::LOOK;
		}
	}
}

void components::camera::configure_camera_player_crosshair(augs::entity_id camera, augs::entity_id player, augs::entity_id crosshair) {
	camera->get<components::camera>().player = player;
	camera->get<components::camera>().crosshair = crosshair;
	camera->get<components::position_copying>().set_target(player);

	crosshair->get<components::crosshair>().parent_camera = camera;
}

components::camera::constraint_output components::camera::get_constrained_crosshair_and_camera_offset(augs::entity_id self) {
	components::camera::constraint_output out;

	auto transform = self->get<components::transform>();

	/* if we set player and crosshair entity targets */
	if (player.alive() && crosshair.alive()) {
		/* skip calculations if no orbit_mode is specified */
		if (orbit_mode != NONE) {
			/* shortcuts */
			vec2i player_pos = player->get<components::transform>().pos;
			vec2i crosshair_pos = crosshair->get<components::crosshair>().base_offset + player_pos;
			out.constrained_crosshair_pos = (crosshair_pos - player_pos);

			out.constrained_crosshair_pos.rotate(transform.rotation, vec2());

			if (orbit_mode == ANGLED) {
				vec2 bound = visible_world_area / 2.f;
				/* save by copy */
				vec2 normalized = out.constrained_crosshair_pos.clamp(bound);
				out.camera_crosshair_offset = normalized.normalize() * angled_look_length;
			}

			if (orbit_mode == LOOK) {
				vec2 bound = max_look_expand + visible_world_area / 2.f;

				/* simple proportion in local frame of reference */
				vec2 camera_offset = (out.constrained_crosshair_pos.clamp(bound) / bound) * max_look_expand;

				out.camera_crosshair_offset = camera_offset.rotate(-transform.rotation, vec2());
			}

			/* rotate dir back */
			out.constrained_crosshair_pos.rotate(-transform.rotation, vec2());
			/* update crosshair so it is snapped to visible area */
			out.constrained_crosshair_base_offset = out.constrained_crosshair_pos;
			out.constrained_crosshair_pos += player_pos;
		}
	}

	return out;
}

void camera_system::resolve_cameras_transforms_and_smoothing() {
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
			
			auto constraints = camera.get_constrained_crosshair_and_camera_offset(e);

			vec2i camera_crosshair_offset;
			
			if (camera.crosshair.alive()) {
				camera_crosshair_offset = constraints.camera_crosshair_offset;
				camera.crosshair->get<components::transform>().pos = constraints.constrained_crosshair_pos;
			}

			components::transform smoothed_camera_transform;
			vec2 smoothed_visible_world_area;

			smoothed_camera_transform = transform;
			smoothed_visible_world_area = camera.visible_world_area;

			smoothed_camera_transform.pos += camera_crosshair_offset;

			if (camera.enable_smoothing) {
				/* variable time step camera smoothing by averaging last position with the current */
				float averaging_constant =
					pow(camera.smoothing_average_factor, camera.averages_per_sec * delta_seconds());
				
				if (camera.dont_smooth_once)
					averaging_constant = 0.0f;

				//if ((transform.pos - camera.last_interpolant).length() < 2.0) camera.last_interpolant = transform.pos;
				//else

				vec2i target = transform.pos + camera_crosshair_offset;
				vec2i smoothed_part = camera_crosshair_offset;

				camera.last_interpolant.pos = camera.last_interpolant.pos * averaging_constant + vec2d(smoothed_part) * (1.0f - averaging_constant);
				camera.last_interpolant.rotation = camera.last_interpolant.rotation * averaging_constant + transform.rotation * (1.0f - averaging_constant);
					
				auto interp = [](float& a, float& b, float averaging_constant){
					a = a * averaging_constant + b * (1.0f - averaging_constant);
				};

				interp(camera.last_ortho_interpolant.x, camera.visible_world_area.x, averaging_constant);
				interp(camera.last_ortho_interpolant.y, camera.visible_world_area.y, averaging_constant);

				/* save smoothing result */
				//if ((smoothed_camera_transform.pos - camera.last_interpolant.pos).length() > 5)
				vec2 calculated_smoothed_pos = target - smoothed_part + camera.last_interpolant.pos;
				int calculated_smoothed_rotation = camera.last_interpolant.rotation;

				//if (vec2i(calculated_smoothed_pos) == vec2i(smoothed_camera_transform.pos))
				//	camera.last_interpolant.pos = smoothed_part;
				//if (int(calculated_smoothed_rotation) == int(smoothed_camera_transform.rotation))
				//	camera.last_interpolant.rotation = smoothed_camera_transform.rotation;
				
				if (calculated_smoothed_pos.compare_abs(smoothed_camera_transform.pos, 1.f))
					camera.last_interpolant.pos = smoothed_part;
				if (std::abs(calculated_smoothed_rotation - smoothed_camera_transform.rotation) < 1.f)
					camera.last_interpolant.rotation = smoothed_camera_transform.rotation;

				smoothed_camera_transform.pos = calculated_smoothed_pos;
				smoothed_camera_transform.rotation = calculated_smoothed_rotation;

				//smoothing_player_pos

				smoothed_visible_world_area = camera.last_ortho_interpolant;

				if (camera.crosshair_follows_interpolant) {
					camera.crosshair->get<components::transform>().pos -= transform.pos - camera.last_interpolant.pos;
				}
			}

			if (camera.player.alive()) {
				vec2 target_value;
				auto maybe_physics = camera.player->find<components::physics>();

				if (maybe_physics) {
					auto player_pos = maybe_physics->get_position();

					if (player_pos != camera.previous_step_player_position) {
						camera.previous_seen_player_position = camera.previous_step_player_position;
						camera.previous_step_player_position = player_pos;
					}

					target_value = (player_pos - camera.previous_seen_player_position) / delta_seconds();

					//maybe_physics->velocity();
					
					if (target_value.length() < camera.smoothing_player_pos.value.length())
						// braking
						camera.smoothing_player_pos.averages_per_sec = 3.5;
					else
						camera.smoothing_player_pos.averages_per_sec = 1.5;
					
					if (target_value.length() > 50)
						target_value.set_length(50);
				}
				else {
					target_value = camera.player->get<components::render>().interpolation_direction();
					target_value.set_length(100);
					camera.smoothing_player_pos.averages_per_sec = 5;
				}

				camera.smoothing_player_pos.target_value = target_value * (-1);
				camera.smoothing_player_pos.tick();
			}

			smoothed_camera_transform.pos = vec2i(smoothed_camera_transform.pos + camera.smoothing_player_pos.value);
			smoothed_visible_world_area = vec2i(smoothed_visible_world_area);

			camera.dont_smooth_once = false;

			shared::drawing_state in;
			in.transformed_visible_world_area_aabb = rects::ltrb<float>::get_aabb_rotated(smoothed_visible_world_area, smoothed_camera_transform.rotation) 
				+ smoothed_camera_transform.pos - smoothed_visible_world_area / 2;
			in.camera_transform = smoothed_camera_transform;
			in.output = &get_renderer();
			in.visible_world_area = smoothed_visible_world_area;
			in.viewport = camera.viewport;

			camera.how_camera_will_render = in;
		}
	}
}

void camera_system::post_render_requests_for_all_cameras() {
	parent_world.get_message_queue<messages::camera_render_request_message>().clear();

	for (auto e : targets) {
		auto& camera = e->get<components::camera>();

		if (camera.enabled) {
			auto& in = camera.how_camera_will_render;

			messages::camera_render_request_message msg;
			msg.camera = e;
			msg.state = in;
			msg.mask = camera.mask;

			parent_world.post_message(msg);
		}
	}
}
