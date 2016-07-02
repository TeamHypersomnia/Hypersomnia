#include "math/vec2.h"
#include "camera_system.h"

#include "game/entity_id.h"
#include "game/components/physics_component.h"
#include "game/components/crosshair_component.h"
#include "game/components/position_copying_component.h"
#include "game/components/crosshair_component.h"
#include "game/components/physics_component.h"
#include "game/messages/intent_message.h"
#include "game/messages/camera_render_request_message.h"
#include "game/detail/state_for_drawing_camera.h"
#include "game/cosmos.h"

#include "game/components/camera_component.h"
#include "game/components/transform_component.h"
#include "graphics/renderer.h"

#include "game/entity_handle.h"
#include "game/step.h"

using namespace augs;

void update_bounds_for_crosshair(components::camera& camera, components::crosshair& crosshair) {
	if (camera.orbit_mode == components::camera::ANGLED)
		crosshair.bounds_for_base_offset = camera.visible_world_area / 2.f;
	if (camera.orbit_mode == components::camera::LOOK)
		crosshair.bounds_for_base_offset = camera.max_look_expand + camera.visible_world_area / 2.f;
}

void camera_system::react_to_input_intents(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	auto events = step.messages.get_queue<messages::intent_message>();

	for (auto it : events) {
		if (cosmos[it.subject].find<components::camera>() == nullptr)
			continue;

		if (it.intent == intent_type::SWITCH_LOOK && it.pressed_flag) {
			auto& camera = cosmos[it.subject].get<components::camera>();
			auto& mode = camera.orbit_mode;

			if (mode == components::camera::LOOK)
				mode = components::camera::ANGLED;
			else mode = components::camera::LOOK;

			auto crosshair = cosmos.get_handle(camera.entity_to_chase)[sub_entity_name::CHARACTER_CROSSHAIR];

			if (crosshair.alive())
				update_bounds_for_crosshair(camera, cosmos[crosshair].get<components::crosshair>());
		}
	}
}

void components::camera::configure_camera_and_character_with_crosshair(entity_handle camera, entity_handle character, entity_handle crosshair) {
	camera.get<components::camera>().entity_to_chase = character;
	camera.get<components::position_copying>().set_target(character);

	update_bounds_for_crosshair(camera.get<components::camera>(), crosshair.get<components::crosshair>());
}

vec2i components::camera::get_camera_offset_due_to_character_crosshair(cosmos& cosmos) const {
	vec2 camera_crosshair_offset;

	if (cosmos[entity_to_chase].dead())
		return vec2i(0, 0);

	auto crosshair_entity = cosmos.get_handle(entity_to_chase)[sub_entity_name::CHARACTER_CROSSHAIR];

	/* if we set player and crosshair entity targets */
	/* skip calculations if no orbit_mode is specified */
	if (crosshair_entity.alive() && orbit_mode != NONE) {
		auto& crosshair = crosshair_entity.get<components::crosshair>();
		camera_crosshair_offset = components::crosshair::calculate_aiming_displacement(crosshair_entity, false);
		
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

void camera_system::resolve_cameras_transforms_and_smoothing(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();

	auto targets_copy = cosmos.get(processing_subjects::WITH_CAMERA);
	/* we sort layers in reverse order to keep layer 0 as topmost and last layer on the bottom */
	std::sort(targets_copy.begin(), targets_copy.end(), [](const entity_handle& a, const entity_handle& b) {
		return a.get<components::camera>().layer > b.get<components::camera>().layer;
	});


	for (auto e : targets_copy) {
		auto& camera = e.get<components::camera>();

		if (camera.enabled) {
			/* we obtain transform as a copy because we'll be now offsetting it by crosshair position */
			auto transform = e.get<components::transform>();
			transform.pos = vec2i(transform.pos);

			vec2i camera_crosshair_offset = camera.get_camera_offset_due_to_character_crosshair(cosmos);

			components::transform smoothed_camera_transform;
			vec2 smoothed_visible_world_area;

			smoothed_camera_transform = transform;
			smoothed_visible_world_area = camera.visible_world_area;

			smoothed_camera_transform.pos += camera_crosshair_offset;

			if (camera.enable_smoothing) {
				/* variable time step camera smoothing by averaging last position with the current */
				float averaging_constant = 1.0f - pow(camera.smoothing_average_factor, camera.averages_per_sec * cosmos.delta.in_seconds());
				
				if (camera.dont_smooth_once)
					averaging_constant = 0.0f;

				//if ((transform.pos - camera.last_interpolant).length() < 2.0) camera.last_interpolant = transform.pos;
				//else

				vec2i target = transform.pos + camera_crosshair_offset;
				vec2i smoothed_part = camera_crosshair_offset;

				camera.last_interpolant.pos = augs::interp(vec2d(camera.last_interpolant.pos), vec2d(smoothed_part), averaging_constant);
				camera.last_interpolant.rotation = augs::interp(camera.last_interpolant.rotation, transform.rotation, averaging_constant);
					
				camera.last_ortho_interpolant.x = camera.visible_world_area.x; //augs::interp(camera.last_ortho_interpolant.x, camera.visible_world_area.x, averaging_constant);
				camera.last_ortho_interpolant.y = camera.visible_world_area.y; //augs::interp(camera.last_ortho_interpolant.y, camera.visible_world_area.y, averaging_constant);

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

			if (cosmos[camera.entity_to_chase].alive()) {
				vec2 target_value;
				auto entity_to_chase = cosmos[camera.entity_to_chase];
				if (entity_to_chase.has<components::physics>()) {
					auto& physics = entity_to_chase.get<components::physics>();

					auto player_pos = physics.get_mass_position();

					if (player_pos != camera.previous_step_player_position) {
						camera.previous_seen_player_position = camera.previous_step_player_position;
						camera.previous_step_player_position = player_pos;
					}

					target_value = (player_pos - camera.previous_seen_player_position) * cosmos.delta.in_milliseconds();
					auto vel = physics.velocity();
					
					if (target_value.length() < camera.smoothing_player_pos.value.length())
						// braking
						camera.smoothing_player_pos.averages_per_sec = 3.5;
					else
						camera.smoothing_player_pos.averages_per_sec = 1.5;
					
					if (target_value.length() > 50)
						target_value.set_length(50);

					// LOG("%x, %x, %x", *(vec2i*)&player_pos, *(vec2i*)&camera.previous_step_player_position, *(vec2i*)&target_value);
				}
				else {
					target_value = cosmos[camera.entity_to_chase].get<components::render>().interpolation_direction();
					target_value.set_length(100);
					camera.smoothing_player_pos.averages_per_sec = 5;
				}

				camera.smoothing_player_pos.target_value = target_value * (-1);
				camera.smoothing_player_pos.tick();
			}

			smoothed_camera_transform.pos = vec2i(smoothed_camera_transform.pos + camera.smoothing_player_pos.value);
			smoothed_visible_world_area = vec2i(smoothed_visible_world_area);

			camera.dont_smooth_once = false;

			state_for_drawing_camera in;
			in.transformed_visible_world_area_aabb = get_aabb_rotated(smoothed_visible_world_area, smoothed_camera_transform.rotation) 
				+ smoothed_camera_transform.pos - smoothed_visible_world_area / 2;
			in.camera_transform = smoothed_camera_transform;
			in.visible_world_area = smoothed_visible_world_area;
			in.associated_character = camera.entity_to_chase;

			camera.how_camera_will_render = in;
		}
	}
}