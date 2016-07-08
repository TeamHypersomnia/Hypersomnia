#include "ingredients.h"
#include "game/entity_handle.h"
#include "game/cosmos.h"

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
#include "game/components/gui_element_component.h"
#include "game/components/name_component.h"
#include "game/components/sentience_component.h"
#include "game/components/particle_effect_response_component.h"
#include "game/components/attitude_component.h"
#include "game/assets/particle_effect_response_id.h"

#include "game/enums/filters.h"
#include "game/enums/party_category.h"

namespace ingredients {
	void wsad_character_setup_movement(entity_handle e) {
		components::movement& movement = e.get<components::movement>();

		movement.add_animation_receiver(e, false);

		movement.input_acceleration_axes.set(1, 1);
		movement.acceleration_length = 10000;
		
		//movement.input_acceleration_axes.set(8000, 8000);
		//movement.acceleration_length = -1;

		movement.max_speed_for_movement_response = 1000;
		movement.braking_damping = 24.5f;
		
		movement.enable_braking_damping = true;
	}

	void wsad_character_physics(entity_handle e) {
		auto& physics_definition = e += components::physics_definition();

		auto& body = physics_definition.body;
		auto& info = physics_definition.new_collider();

		info.shape.from_renderable(e);

		info.filter = filters::controlled_character();
		info.density = 1.0;
		body.fixed_rotation = false;
		body.angled_damping = true;
		body.angle_motor_force_multiplier = 1.0;

		body.linear_damping = 20;

		wsad_character_setup_movement(e);
	}

	void wsad_character_legs(entity_handle legs, entity_handle player) {
		components::sprite sprite;
		components::render render;
		components::animation animation;
		components::transform transform;
	}

	void wsad_character(entity_handle e, entity_handle crosshair_entity) {
		auto& sprite = e += components::sprite();
		auto& render = e += components::render();
		auto& animation = e += components::animation();
		auto& animation_response = e += components::animation_response();
		auto& transform = e += components::transform();
		auto& movement = e += components::movement();
		auto& rotation_copying = e += components::rotation_copying();
		auto& detector = e += components::trigger_query_detector();
		auto& driver = e += components::driver();
		auto& force_joint = e += components::force_joint();
		auto& sentience = e += components::sentience();
		e += components::position_copying(); // used when it is an astral body
		auto& particle_response = e += components::particle_effect_response({ assets::particle_effect_response_id::CHARACTER_RESPONSE });
		auto& attitude = e += components::attitude();

		attitude.parties = party_category::METROPOLIS_CITIZEN;
		attitude.hostile_parties = party_category::RESISTANCE_CITIZEN;

		particle_response.modifier.colorize = augs::red;
		particle_response.modifier.scale_lifetimes = 1.5f;

		sentience.aimpunch.offsets = {
				{ vec2().set_from_degrees(0) },
				{ vec2().set_from_degrees(6) },
				{ vec2().set_from_degrees(-6) },
				{ vec2().set_from_degrees(7) },
				{ vec2().set_from_degrees(-7) },
				{ vec2().set_from_degrees(-9) },
				{ vec2().set_from_degrees(11) },
				{ vec2().set_from_degrees(-12) },
				{ vec2().set_from_degrees(-4) },
				{ vec2().set_from_degrees(11) },
				{ vec2().set_from_degrees(23) },
				{ vec2().set_from_degrees(53) },
				{ vec2().set_from_degrees(10) },
				{ vec2().set_from_degrees(-30) },
				{ vec2().set_from_degrees(-60) },
				{ vec2().set_from_degrees(-70) },
				{ vec2().set_from_degrees(-80) },
				{ vec2().set_from_degrees(-20) },
				{ vec2().set_from_degrees(50) },
				{ vec2().set_from_degrees(80) },
				{ vec2().set_from_degrees(120) },
				{ vec2().set_from_degrees(60) },
				{ vec2().set_from_degrees(20) },
				{ vec2().set_from_degrees(40) },
				{ vec2().set_from_degrees(20) },
				{ vec2().set_from_degrees(-40) },
		};

		sentience.aimpunch.offsets = {
			{ vec2().set_from_degrees(1) },
			{ vec2().set_from_degrees(1) },
			{ vec2().set_from_degrees(2) },
			{ vec2().set_from_degrees(2) },
			{ vec2().set_from_degrees(3) },
			{ vec2().set_from_degrees(-1) },
			{ vec2().set_from_degrees(1) },
			{ vec2().set_from_degrees(-1) },
			{ vec2().set_from_degrees(-2) },
			{ vec2().set_from_degrees(-1) },
			{ vec2().set_from_degrees(-1) },
			{ vec2().set_from_degrees(-2) },
			{ vec2().set_from_degrees(-3) },
			{ vec2().set_from_degrees(-4) },
			{ vec2().set_from_degrees(-5) },
			{ vec2().set_from_degrees(-3) },
			{ vec2().set_from_degrees(-2) },
			{ vec2().set_from_degrees(-2) },
			{ vec2().set_from_degrees(-1) },
			{ vec2().set_from_degrees(1) },
			{ vec2().set_from_degrees(2) },
			{ vec2().set_from_degrees(3) },
			{ vec2().set_from_degrees(2) },
			{ vec2().set_from_degrees(3) },
			{ vec2().set_from_degrees(4) },
			{ vec2().set_from_degrees(5) },
		};

		sentience.aimpunch.repeat_last_n_offsets = 20;
		sentience.aimpunch.scale = 150.0;
		sentience.aimpunch.single_cooldown_duration_ms= 200.0;

		sentience.health.value = 100.0;
		sentience.health.maximum = 100.0;

		e.deactivate(force_joint);

		detector.spam_trigger_requests_when_detection_intented = true;

		force_joint.force_towards_chased_entity = 85000.f;
		force_joint.distance_when_force_easing_starts = 10.f;
		force_joint.power_of_force_easing_multiplier = 2.f;

		driver.density_multiplier_while_driving = 0.02f;

		movement.standard_linear_damping = 20.f;
		// driver.linear_damping_while_driving = 4.f;

		e.map_sub_entity(sub_entity_name::CHARACTER_CROSSHAIR, crosshair_entity);
		
		animation_response.response = assets::animation_response_id::TORSO_SET;

		sprite.set(assets::texture_id::TEST_PLAYER, rgba(255, 255, 255, 255));

		render.layer = render_layer::DYNAMIC_BODY;

		rotation_copying.target = crosshair_entity;
		rotation_copying.look_mode = components::rotation_copying::look_type::POSITION;
		rotation_copying.use_physical_motor = true;
		rotation_copying.colinearize_item_in_hand = true;

		wsad_character_setup_movement(e);
	}

	void wsad_character_corpse(entity_handle e) {
		auto& sprite = e += components::sprite();
		auto& render = e += components::render();
		auto& transform = e += components::transform();

		sprite.set(assets::texture_id::DEAD_TORSO, rgba(255, 255, 255, 255));
		render.layer = render_layer::CORPSES;

		auto& physics_definition = e += components::physics_definition();

		auto& body = physics_definition.body;
		auto& info = physics_definition.new_collider();

		info.shape.from_renderable(e);

		info.filter = filters::corpse();
		info.density = 1.0;

		body.linear_damping = 6.5;
	}

	void inject_window_input_to_character(entity_handle next_character, entity_handle camera) {
		auto previously_controlled_character = next_character.make_handle(camera.get<components::camera>().entity_to_chase);

		if (previously_controlled_character.alive()) {
			previously_controlled_character.skip_processing_in(processing_subjects::WITH_INPUT_RECEIVER);
			previously_controlled_character.skip_processing_in(processing_subjects::WITH_GUI_ELEMENT);

			auto crosshair = previously_controlled_character[sub_entity_name::CHARACTER_CROSSHAIR];
			crosshair.skip_processing_in(processing_subjects::WITH_INPUT_RECEIVER);

			previously_controlled_character.map_associated_entity(associated_entity_name::WATCHING_CAMERA, entity_id());
		}

		auto crosshair = next_character[sub_entity_name::CHARACTER_CROSSHAIR];

		next_character.map_associated_entity(associated_entity_name::WATCHING_CAMERA, camera);

		if (next_character.find<components::gui_element>() == nullptr)
			next_character.add(components::gui_element());

		if (next_character.find<components::input_receiver>() == nullptr)
			next_character.add<components::input_receiver>();

		if (crosshair.find<components::input_receiver>() == nullptr)
			crosshair.add<components::input_receiver>();

		next_character.unskip_processing_in(processing_subjects::WITH_INPUT_RECEIVER);
		next_character.unskip_processing_in(processing_subjects::WITH_GUI_ELEMENT);
		crosshair.unskip_processing_in(processing_subjects::WITH_INPUT_RECEIVER);

		components::camera::configure_camera_and_character_with_crosshair(camera, next_character, crosshair);
	}
}

namespace prefabs {
	entity_handle create_character(cosmos world, vec2 pos) {
		auto character = world.create_entity("player_unnamed");

		name_entity(character, entity_name::PERSON);

		auto crosshair = create_character_crosshair(world);
		crosshair.get<components::crosshair>().character_entity_to_chase = character;

		ingredients::wsad_character(character, crosshair);

		character.get<components::transform>().pos = pos;

		ingredients::wsad_character_physics(character);

		ingredients::character_inventory(character);

		auto corpse_of_sentience = world.create_definition_entity("corpse_of_sentience");
		name_entity(corpse_of_sentience, entity_name::CORPSE);
		ingredients::wsad_character_corpse(corpse_of_sentience);

		character->map_sub_entity(sub_entity_name::CORPSE_OF_SENTIENCE, corpse_of_sentience);

		return character;
	}

	entity_handle create_character_crosshair(cosmos world) {
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

			sprite.set(assets::texture_id::TEST_CROSSHAIR, rgba(0, 255, 0, 0));

			auto& render = *recoil += components::render();
			render.layer = render_layer::OVER_CROSSHAIR;
			render.interpolate = true;
			render.snap_interpolation_when_close = false;
			ingredients::make_always_visible(recoil);

			auto& body = physics_definition.body;
			auto& info = physics_definition.new_collider();

			info.shape.from_renderable(recoil);

			info.filter = filters::renderable();
			//info.filter.categoryBits = 0;
			info.density = 0.1;
			info.sensor = true;

			body.linear_damping = 5;
			body.angular_damping = 5;

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