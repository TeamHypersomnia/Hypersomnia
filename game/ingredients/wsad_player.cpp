#include "ingredients.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "game/components/position_copying_component.h"
#include "game/components/camera_component.h"
#include "game/components/input_receiver_component.h"
#include "game/components/crosshair_component.h"
#include "game/components/sprite_component.h"
#include "game/components/movement_component.h"
#include "game/components/rotation_copying_component.h"
#include "game/components/animation_component.h"
#include "game/components/animation_response_component.h"
#include "game/components/physics_component.h"
#include "game/components/trigger_query_detector_component.h"
#include "game/components/driver_component.h"
#include "game/components/force_joint_component.h"
#include "game/components/physics_definition_component.h"
#include "game/components/gui_element_component.h"
#include "game/components/name_component.h"

#include "game/globals/filters.h"
#include "game/globals/input_profiles.h"
#include "game/settings.h"

namespace ingredients {
	void wsad_character_setup_movement(augs::entity_id e) {
		components::movement& movement = e->get<components::movement>();

		movement.add_animation_receiver(e, false);

		movement.input_acceleration_axes.set(1, 1);
		movement.acceleration_length = 10000;
		
		//movement.input_acceleration_axes.set(8000, 8000);
		//movement.acceleration_length = -1;

		movement.max_speed_for_movement_response = 1000;
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
		body.angle_motor_force_multiplier = 1.0;

		body.linear_damping = 20;

		wsad_character_setup_movement(e);
	}

	void wsad_character_legs(augs::entity_id legs, augs::entity_id player) {
		components::sprite sprite;
		components::render render;
		components::animation animation;
		components::transform transform;
	}

	void wsad_character(augs::entity_id e, augs::entity_id crosshair_entity) {
		auto& sprite = *e += components::sprite();
		auto& render = *e += components::render();
		auto& animation = *e += components::animation();
		auto& animation_response = *e += components::animation_response();
		auto& transform = *e += components::transform();
		auto& movement = *e += components::movement();
		auto& rotation_copying = *e += components::rotation_copying();
		auto& detector = *e += components::trigger_query_detector();
		auto& driver = *e += components::driver();
		auto& force_joint = *e += components::force_joint();
		e->disable(force_joint);

		detector.spam_trigger_requests_when_detection_intented = true;

		force_joint.force_towards_chased_entity = 85000.f;
		force_joint.distance_when_force_easing_starts = 10.f;
		force_joint.power_of_force_easing_multiplier = 2.f;

		driver.density_multiplier_while_driving = 0.02f;

		movement.standard_linear_damping = 20.f;
		// driver.linear_damping_while_driving = 4.f;

		e->map_sub_entity(sub_entity_name::CHARACTER_CROSSHAIR, crosshair_entity);
		
		animation_response.response = assets::animation_response_id::TORSO_SET;

		sprite.set(assets::texture_id::TEST_PLAYER, rgba(255, 255, 255, 255));

		render.layer = render_layer::CHARACTER;

		rotation_copying.target = crosshair_entity;
		rotation_copying.look_mode = components::rotation_copying::look_type::POSITION;
		rotation_copying.use_physical_motor = true;
		rotation_copying.colinearize_item_in_hand = true;

		wsad_character_setup_movement(e);
	}

	void inject_window_input_to_character(augs::entity_id next_character, augs::entity_id camera) {
		auto previously_controlled_character = camera->get<components::camera>().entity_to_chase;

		if (previously_controlled_character.alive()) {
			previously_controlled_character->disable<components::input_receiver>();
			previously_controlled_character->disable<components::gui_element>();

			auto crosshair = previously_controlled_character[sub_entity_name::CHARACTER_CROSSHAIR];
			crosshair->disable<components::input_receiver>();

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

namespace prefabs {
	augs::entity_id create_character(augs::world& world, vec2 pos) {
		auto character = world.create_entity("player_unnamed");
		name_entity(character, entity_name::PERSON);

		ingredients::wsad_character(character, create_character_crosshair(world));

		character->get<components::transform>().pos = pos;

		ingredients::wsad_character_physics(character);

		ingredients::character_inventory(character);

		return character;
	}

	augs::entity_id create_character_crosshair(augs::world& world) {
		auto root = world.create_entity("crosshair");
		auto recoil = world.create_entity("crosshair_recoil_body");
		auto zero_target = world.create_entity("zero_target");

		{
			auto& sprite = *root += components::sprite();
			auto& render = *root += components::render();
			auto& transform = *root += components::transform();
			auto& crosshair = *root += components::crosshair();

			sprite.set(assets::texture_id::TEST_CROSSHAIR, rgba(0, 255, 0, 255));

			render.layer = render_layer::CROSSHAIR;
			render.interpolate = false;

			crosshair.sensitivity.set(3, 3);

			ingredients::make_always_visible(root);
		}

		{
			auto& transform = *recoil += components::transform();
			auto& physics_definition = *recoil += components::physics_definition();
			auto& force_joint = *recoil += components::force_joint();
			*zero_target += components::transform();

			auto& sprite = *recoil += components::sprite();

			sprite.set(assets::texture_id::TEST_CROSSHAIR, rgba(0, 255, 0, 255));

			if (DEBUG_DRAW_RECOIL_CROSSHAIR) {
				auto& render = *recoil += components::render();
				render.layer = render_layer::OVER_CROSSHAIR;
				render.interpolate = true;
			}

			auto& body = physics_definition.body;
			auto& info = physics_definition.new_fixture();

			info.from_renderable(recoil);

			info.filter = filters::renderable();
			//info.filter.categoryBits = 0;
			info.density = 0.1;
			info.sensor = true;
			body.fixed_rotation = true;

			body.linear_damping = 5;

			force_joint.chased_entity = zero_target;
			//force_joint.consider_rotation = false;
			//force_joint.distance_when_force_easing_starts = 10.f;
			//force_joint.force_towards_chased_entity = 1000.f;
			//force_joint.power_of_force_easing_multiplier = 1.f;
			force_joint.divide_transform_mode = true;
		}

		root->map_sub_entity(sub_entity_name::CROSSHAIR_RECOIL_BODY, recoil);

		return root;
	}
}