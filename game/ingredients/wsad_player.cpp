#include "ingredients.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/components/position_copying_component.h"

#include "game/components/crosshair_component.h"
#include "game/components/sprite_component.h"
#include "game/components/movement_component.h"
#include "game/components/rotation_copying_component.h"
#include "game/components/animation_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/animation_response_component.h"
#include "game/components/physics_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/trigger_query_detector_component.h"
#include "game/components/driver_component.h"
#include "game/components/force_joint_component.h"
#include "game/detail/gui/character_gui.h"
#include "game/components/name_component.h"
#include "game/components/sentience_component.h"
#include "game/components/particle_effect_response_component.h"
#include "game/components/sound_response_component.h"
#include "game/components/attitude_component.h"
#include "game/components/dynamic_tree_node_component.h"
#include "game/components/flags_component.h"
#include "game/assets/particle_effect_response_id.h"
#include "game/systems_stateless/particles_existence_system.h"

#include "game/enums/filters.h"
#include "game/enums/party_category.h"

namespace ingredients {
	void add_wsad_character_setup_movement(const entity_handle e) {
		components::movement& movement = e.get<components::movement>();

		movement.add_animation_receiver(e, false);

		movement.input_acceleration_axes.set(1, 1);
		movement.acceleration_length = 8000;
		
		//movement.input_acceleration_axes.set(8000, 8000);
		//movement.acceleration_length = -1;

		movement.max_speed_for_movement_response = 1000;
		movement.braking_damping = 12.5f;
		
		movement.enable_braking_damping = true;
	}

	void add_wsad_character_physics(const entity_handle e) {
		components::physics body;
		components::special_physics special;
		components::fixtures colliders;

		auto& info = colliders.new_collider();

		info.shape.from_renderable(e);

		info.filter = filters::controlled_character();
		info.density = 1.0;
		body.fixed_rotation = false;
		body.angled_damping = true;

		body.linear_damping = 0;

		add_wsad_character_setup_movement(e);

		e += body;
		e += special;
		e += colliders;
		e.get<components::fixtures>().set_owner_body(e);
	}

	void add_wsad_character_legs(const entity_handle legs, const entity_handle player) {
		components::sprite sprite;
		components::render render;
		components::animation animation;
	}

	void add_wsad_character(const entity_handle e, const entity_handle crosshair_entity, const assets::animation_response_id torso_set) {
		auto& sprite = e += components::sprite();
		auto& render = e += components::render();
		auto& animation = e += components::animation();
		auto& animation_response = e += components::animation_response();
		auto& movement = e += components::movement();
		auto& rotation_copying = e += components::rotation_copying();
		auto& detector = e += components::trigger_query_detector();
		auto& driver = e += components::driver();
		auto& force_joint = e += components::force_joint();
		auto& sentience = e += components::sentience();
		e += components::position_copying(); // used when it is an astral body
		auto& particle_response = e += components::particle_effect_response { assets::particle_effect_response_id::CHARACTER_RESPONSE };
		
		{
			auto& sound_response = e += components::sound_response();
			sound_response.response = assets::sound_response_id::CHARACTER_RESPONSE;
		}
		
		auto& attitude = e += components::attitude();
		auto& processing = e += components::processing();
		e.set_flag(entity_flag::IS_PAST_CONTAGIOUS);

		attitude.parties = party_category::METROPOLIS_CITIZEN;
		attitude.hostile_parties = party_category::RESISTANCE_CITIZEN;

		particle_response.modifier.colorize = red;
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

		processing.disable_in(processing_subjects::WITH_FORCE_JOINT);

		detector.spam_trigger_requests_when_detection_intented = true;

		force_joint.force_towards_chased_entity = 92000.f;
		force_joint.distance_when_force_easing_starts = 10.f;
		force_joint.power_of_force_easing_multiplier = 2.f;

		driver.density_multiplier_while_driving = 0.02f;

		movement.standard_linear_damping = 20.f;
		// driver.linear_damping_while_driving = 4.f;

		e.map_sub_entity(sub_entity_name::CHARACTER_CROSSHAIR, crosshair_entity);

		animation_response.response = torso_set;

		sprite.set(assets::texture_id::TEST_PLAYER, rgba(255, 255, 255, 255));

		render.layer = render_layer::SMALL_DYNAMIC_BODY;

		rotation_copying.target = crosshair_entity;
		rotation_copying.look_mode = components::rotation_copying::look_type::POSITION;
		rotation_copying.colinearize_item_in_hand = true;

		add_wsad_character_setup_movement(e);
	}
}

namespace prefabs {
	entity_handle create_character(
		cosmos& world, 
		const components::transform spawn_transform, 
		const vec2i screen_size, 
		const std::string name, 
		const assets::animation_response_id torso_set
	) {
		const auto character = world.create_entity(name);

		name_entity(character, entity_name::PERSON);

		const auto crosshair = create_character_crosshair(world, screen_size);
		crosshair.get<components::crosshair>().character_entity_to_chase = character;
		crosshair.set_logic_transform(spawn_transform.pos);

		ingredients::add_wsad_character(character, crosshair, torso_set);
		
		ingredients::add_wsad_character_physics(character);

		character.get<components::physics>().set_transform(spawn_transform);

		ingredients::add_character_inventory(character);

		const auto corpse_of_sentience = world.create_entity("corpse_of_sentience");
		name_entity(corpse_of_sentience, entity_name::CORPSE);

		character.map_sub_entity(sub_entity_name::CORPSE_OF_SENTIENCE, corpse_of_sentience);

		{
			messages::create_particle_effect effect;
			effect.place_of_birth = character.get_logic_transform();

			effect.input.effect = assets::particle_effect_id::HEALTH_DAMAGE_SPARKLES;
			effect.subject = character;
			effect.input.delete_entity_after_effect_lifetime = false;

			const auto particles = particles_existence_system().create_particle_effect_entity(character.get_cosmos(), effect);

			particles.add_standard_components();
			character.add_sub_entity(particles);
			character.get<components::sentience>().health_damage_particles = particles;
			components::particles_existence::deactivate(particles);
		}

		character.add_standard_components();

		// LOG("Character mass: %x", character.get<components::physics>().get_mass());
		return character;
	}

	entity_handle create_character_crosshair(cosmos& world, const vec2i screen_size) {
		auto root = world.create_entity("crosshair");
		auto recoil = world.create_entity("crosshair_recoil_body");
		auto zero_target = world.create_entity("zero_target");

		{
			auto& sprite = root += components::sprite();
			auto& render = root += components::render();
			auto& transform = root += components::transform();
			auto& crosshair = root += components::crosshair();
			auto& processing = root += components::processing();
			
			sprite.set(assets::texture_id::TEST_CROSSHAIR, rgba(255, 255, 255, 255));

			render.layer = render_layer::CROSSHAIR;

			crosshair.base_offset.set(-20, 0);
			crosshair.sensitivity.set(3, 3);
			crosshair.visible_world_area.set(1920, 1080);
			crosshair.update_bounds();

			ingredients::make_always_visible(root);
		}

		{
			auto& force_joint = recoil += components::force_joint();
			zero_target += components::transform();
			components::physics body;
			components::fixtures colliders;

			auto& sprite = recoil += components::sprite();

			sprite.set(assets::texture_id::TEST_CROSSHAIR, rgba(0, 255, 0, 0));

			auto& render = recoil += components::render();
			render.layer = render_layer::OVER_CROSSHAIR;
			ingredients::make_always_visible(recoil);

			auto& info = colliders.new_collider();

			info.shape.from_renderable(recoil);

			info.filter = b2Filter();
			//info.filter.categoryBits = 0;
			info.density = 0.1f;
			info.sensor = true;

			body.linear_damping = 5;
			body.angular_damping = 5;

			force_joint.chased_entity = zero_target;
			//force_joint.consider_rotation = false;
			//force_joint.distance_when_force_easing_starts = 10.f;
			//force_joint.force_towards_chased_entity = 1000.f;
			//force_joint.power_of_force_easing_multiplier = 1.f;
			force_joint.divide_transform_mode = true;

			recoil += body;
			recoil += colliders;
			recoil.get<components::fixtures>().set_owner_body(recoil);
		}

		root.map_sub_entity(sub_entity_name::CROSSHAIR_RECOIL_BODY, recoil);
		
		root.add_standard_components();
		recoil.add_standard_components();
		zero_target.add_standard_components();

		return root;
	}
}