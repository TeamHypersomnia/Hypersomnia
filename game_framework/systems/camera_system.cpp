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
#include "../detail/state_for_drawing.h"
#include "entity_system/world.h"

void update_bounds_for_crosshair(components::camera& camera, components::crosshair& crosshair) {
	if (camera.orbit_mode == components::camera::ANGLED)
		crosshair.bounds_for_base_offset = camera.visible_world_area / 2.f;
	if (camera.orbit_mode == components::camera::LOOK)
		crosshair.bounds_for_base_offset = camera.max_look_expand + camera.visible_world_area / 2.f;
}

void camera_system::react_to_input_intents() {
	auto events = parent_world.get_message_queue<messages::intent_message>();

	for (auto it : events) {
		if (it.subject->find<components::camera>() == nullptr)
			continue;

		if (it.intent == intent_type::SWITCH_LOOK && it.pressed_flag) {
			auto& camera = it.subject->get<components::camera>();
			auto& mode = camera.orbit_mode;

			if (mode == components::camera::LOOK)
				mode = components::camera::ANGLED;
			else mode = components::camera::LOOK;

			auto crosshair = camera.entity_to_chase[sub_entity_name::CHARACTER_CROSSHAIR];

			if (crosshair.alive())
				update_bounds_for_crosshair(camera, crosshair->get<components::crosshair>());
		}
	}
}

void components::camera::configure_camera_and_character_with_crosshair(augs::entity_id camera, augs::entity_id character, augs::entity_id crosshair) {
	camera->get<components::camera>().entity_to_chase = character;
	camera->get<components::position_copying>().set_target(character);
	crosshair->get<components::crosshair>().character_entity_to_chase = character;

	update_bounds_for_crosshair(camera->get<components::camera>(), crosshair->get<components::crosshair>());
}

vec2i components::camera::get_camera_offset_due_to_character_crosshair(augs::entity_id self) {
	vec2 camera_crosshair_offset;

	if (entity_to_chase.dead())
		return vec2i(0, 0);

	auto crosshair_entity = entity_to_chase[sub_entity_name::CHARACTER_CROSSHAIR];

	/* if we set player and crosshair entity targets */
	/* skip calculations if no orbit_mode is specified */
	if (crosshair_entity.alive() && orbit_mode != NONE) {
		auto& crosshair = crosshair_entity->get<components::crosshair>();

		camera_crosshair_offset = crosshair.base_offset;

		if (orbit_mode == ANGLED)
			camera_crosshair_offset.set_length(angled_look_length);

		if (orbit_mode == LOOK) {
			/* simple proportion */
			camera_crosshair_offset /= crosshair.bounds_for_base_offset;
			camera_crosshair_offset *= max_look_expand;
		}

	}

	return camera_crosshair_offset;
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

			vec2i camera_crosshair_offset = camera.get_camera_offset_due_to_character_crosshair(e);

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
					
				camera.last_ortho_interpolant.x = augs::interp(camera.last_ortho_interpolant.x, camera.visible_world_area.x, averaging_constant);
				camera.last_ortho_interpolant.y = augs::interp(camera.last_ortho_interpolant.y, camera.visible_world_area.y, averaging_constant);

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
			}

			if (camera.entity_to_chase.alive()) {
				vec2 target_value;
				auto maybe_physics = camera.entity_to_chase->find<components::physics>();

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
					target_value = camera.entity_to_chase->get<components::render>().interpolation_direction();
					target_value.set_length(100);
					camera.smoothing_player_pos.averages_per_sec = 5;
				}

				camera.smoothing_player_pos.target_value = target_value * (-1);
				camera.smoothing_player_pos.tick();
			}

			smoothed_camera_transform.pos = vec2i(smoothed_camera_transform.pos + camera.smoothing_player_pos.value);
			smoothed_visible_world_area = vec2i(smoothed_visible_world_area);

			camera.dont_smooth_once = false;

			shared::state_for_drawing_camera in;
			in.transformed_visible_world_area_aabb = get_aabb_rotated(smoothed_visible_world_area, smoothed_camera_transform.rotation) 
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
