#include "ingredients.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "game_framework/components/position_copying_component.h"
#include "game_framework/components/camera_component.h"
#include "game_framework/components/input_receiver_component.h"
#include "game_framework/components/crosshair_component.h"
#include "game_framework/components/sprite_component.h"
#include "game_framework/components/movement_component.h"
#include "game_framework/components/rotation_copying_component.h"
#include "game_framework/components/animation_component.h"
#include "game_framework/components/animation_response_component.h"
#include "game_framework/components/physics_component.h"
#include "game_framework/components/trigger_detector_component.h"
#include "game_framework/components/driver_component.h"
#include "game_framework/components/force_joint_component.h"
#include "game_framework/components/physics_definition_component.h"
#include "game_framework/components/gui_element_component.h"

#include "game_framework/globals/filters.h"
#include "game_framework/globals/input_profiles.h"

namespace ingredients {
	void wsad_character_setup_movement(augs::entity_id e) {
		components::movement& movement = e->get<components::movement>();

		movement.add_animation_receiver(e, false);

		movement.input_acceleration_axes.set(1, 1);
		movement.acceleration_length = 10000;
		
		//movement.input_acceleration_axes.set(8000, 8000);
		//movement.acceleration_length = -1;

		movement.max_speed_animation = 1000;
		movement.braking_damping = 24.5f;
		
		movement.enable_braking_damping = true;
	}

	void wsad_character_physics(augs::entity_id e) {
		auto& physics_definition = *e += components::physics_definition();

		auto& body = physics_definition.body;
		auto& info = physics_definition.new_fixture();

		info.from_renderable(e);

		info.filter = filters::controlled_character();
		info.density = 1.0;
		body.fixed_rotation = false;
		body.angled_damping = true;

		body.linear_damping = 20;

		wsad_character_setup_movement(e);
	}

	void wsad_character_legs(augs::entity_id legs, augs::entity_id player) {
		components::sprite sprite;
		components::render render;
		components::animation animation;
		components::transform transform;
	}

	void wsad_character_crosshair(augs::entity_id e) {
		components::sprite sprite;
		components::render render;
		components::transform transform;
		components::crosshair crosshair;

		sprite.set(assets::texture_id::TEST_CROSSHAIR, rgba(255, 0, 0, 255));

		render.layer = render_layer::CROSSHAIR;
		render.interpolate = false;

		crosshair.sensitivity.set(3, 3);

		e->add(render);
		e->add(sprite);
		e->add(transform);
		e->add(crosshair);

		ingredients::make_always_visible(e);
	}

	void wsad_character(augs::entity_id e, augs::entity_id crosshair_entity) {
		auto& sprite = *e += components::sprite();
		auto& render = *e += components::render();
		auto& animation = *e += components::animation();
		auto& animation_response = *e += components::animation_response();
		auto& transform = *e += components::transform();
		auto& movement = *e += components::movement();
		auto& rotation_copying = *e += components::rotation_copying();
		auto& detector = *e += components::trigger_detector();
		auto& driver = *e += components::driver();
		auto& force_joint = *e += components::force_joint();
		e->disable(force_joint);

		force_joint.force_towards_chased_entity = 85000.f;
		force_joint.distance_when_force_easing_starts = 20.f;
		force_joint.power_of_force_easing_multiplier = 1.f;

		driver.density_while_driving = 0.02f;
		driver.standard_density = 0.6f;

		movement.standard_linear_damping = 20.f;
		// driver.linear_damping_while_driving = 4.f;

		e[sub_entity_name::CHARACTER_CROSSHAIR] = crosshair_entity;
		
		animation_response.response = assets::animation_response_id::TORSO_SET;

		sprite.set(assets::texture_id::TEST_PLAYER, rgba(255, 255, 255, 255));

		render.layer = render_layer::CHARACTER;

		rotation_copying.target = crosshair_entity;
		rotation_copying.look_mode = components::rotation_copying::look_type::POSITION;
		rotation_copying.use_physical_motor = true;
		
		wsad_character_setup_movement(e);
	}

	void inject_window_input_to_character(augs::entity_id next_character, augs::entity_id camera) {
		auto previously_controlled_character = camera->get<components::camera>().entity_to_chase;

		if (previously_controlled_character.alive()) {
			previously_controlled_character->disable<components::input_receiver>();
			previously_controlled_character->disable<components::gui_element>();

			previously_controlled_character[sub_entity_name::CHARACTER_CROSSHAIR]->disable<components::input_receiver>();

			previously_controlled_character[associated_entity_name::WATCHING_CAMERA].unset();
		}

		auto crosshair = next_character[sub_entity_name::CHARACTER_CROSSHAIR];

		next_character[associated_entity_name::WATCHING_CAMERA] = camera;

		if (next_character->find<components::gui_element>() == nullptr)
			next_character->add(components::gui_element());

		if (next_character->find<components::input_receiver>() == nullptr)
			next_character->add(input_profiles::character());

		if (crosshair->find<components::input_receiver>() == nullptr)
			crosshair->add(input_profiles::crosshair());

		next_character->enable<components::input_receiver>();
		next_character->enable<components::gui_element>();
		crosshair->enable<components::input_receiver>();

		components::camera::configure_camera_and_character_with_crosshair(camera, next_character, crosshair);
	}
}