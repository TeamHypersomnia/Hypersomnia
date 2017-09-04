#include "ingredients.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/assets/all_assets.h"

#include "game/components/position_copying_component.h"

#include "game/components/crosshair_component.h"
#include "game/components/sprite_component.h"
#include "game/components/movement_component.h"
#include "game/components/rotation_copying_component.h"
#include "game/components/animation_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/driver_component.h"
#include "game/components/force_joint_component.h"
#include "game/view/game_gui/elements/character_gui.h"
#include "game/components/name_component.h"
#include "game/components/sentience_component.h"
#include "game/components/attitude_component.h"
#include "game/components/tree_of_npo_node_component.h"
#include "game/components/flags_component.h"
#include "game/components/shape_polygon_component.h"
#include "game/systems_stateless/particles_existence_system.h"

#include "game/enums/filters.h"
#include "game/enums/party_category.h"
#include "game/detail/inventory/inventory_utils.h"

namespace ingredients {
	void add_character_movement(const entity_handle e) {
		components::movement& movement = e.get<components::movement>();

		movement.input_acceleration_axes.set(1, 1);
		movement.acceleration_length = 10000;
		
		//movement.input_acceleration_axes.set(8000, 8000);
		//movement.acceleration_length = -1;

		movement.max_speed_for_movement_event = 1000;
		movement.braking_damping = 12.5f;
		
		movement.enable_braking_damping = true;
	}

	void add_character_head_physics(const logic_step step, const entity_handle e, const components::transform spawn_transform) {
		components::rigid_body body;
		body.fixed_rotation = false;
		body.angled_damping = true;
		body.allow_sleep = false;
		const auto si = step.cosm.get_si();

		body.set_transform(si, spawn_transform);

		components::special_physics special;

		add_character_movement(e);

		e += body;
		e += special;

		e.add_shape_component_from_renderable(
			step
		);

		components::fixtures group;

		group.filter = filters::controlled_character();
		group.density = 1.0;

		e  += group;
		e.get<components::fixtures>().set_owner_body(e);
	}

	void add_character_legs(const entity_handle legs, const entity_handle player) {
		components::sprite sprite;
		components::render render;
		components::animation animation;
	}

	void add_character(const all_logical_assets& metas, const entity_handle e, const entity_handle crosshair_entity) {
		auto& sprite = e += components::sprite();
		auto& render = e += components::render();
		auto& animation = e += components::animation();
		auto& movement = e += components::movement();
		auto& rotation_copying = e += components::rotation_copying();
		auto& driver = e += components::driver();
		components::motor_joint force_joint;
		auto& sentience = e += components::sentience();
		e += components::position_copying(); // used when it is an astral body
		
		auto& attitude = e += components::attitude();
		auto& processing = e += components::processing();
		e.set_flag(entity_flag::IS_PAST_CONTAGIOUS);

		attitude.parties = party_category::METROPOLIS_CITIZEN;
		attitude.hostile_parties = party_category::RESISTANCE_CITIZEN;

		sentience.health_decrease_particles.id = assets::particle_effect_id::HEALTH_DAMAGE_SPARKLES;
		sentience.health_decrease_particles.modifier.colorize = red;
		sentience.health_decrease_particles.modifier.scale_lifetimes = 1.5f;

		sentience.health_decrease_sound.id = assets::sound_buffer_id::IMPACT;
		sentience.death_sound.id = assets::sound_buffer_id::DEATH;

		sentience.get<health_meter_instance>().set_value(100);
		sentience.get<health_meter_instance>().set_maximum_value(100);

		sentience.loss_of_consciousness_sound.id = assets::sound_buffer_id::DEATH;
		sentience.consciousness_decrease_sound.id = assets::sound_buffer_id::IMPACT;

		processing.disable_in(processing_subjects::WITH_FORCE_JOINT);

		force_joint.max_force = 800000.f;
		force_joint.max_torque = 2000.f;
		force_joint.correction_factor = 0.8f;
		force_joint.activated = false;
		e += force_joint;
		//force_joint.force_towards_chased_entity = 92000.f;
		//force_joint.distance_when_force_easing_starts = 10.f;
		//force_joint.power_of_force_easing_multiplier = 2.f;

		driver.density_multiplier_while_driving = 0.02f;

		movement.standard_linear_damping = 20.f;
		// driver.linear_damping_while_driving = 4.f;

		e.map_child_entity(child_entity_name::CHARACTER_CROSSHAIR, crosshair_entity);

		sprite.set(assets::game_image_id::STANDARD_HEAD, metas);

		render.layer = render_layer::SMALL_DYNAMIC_BODY;

		rotation_copying.target = crosshair_entity;
		rotation_copying.look_mode = components::rotation_copying::look_type::POSITION;
		rotation_copying.colinearize_item_in_hand = true;

		add_character_movement(e);
	}
}

namespace prefabs {
	entity_handle create_sample_complete_character(
		const logic_step step, 
		const components::transform spawn_transform, 
		const std::string name,
		const int create_arm_count
	) {
		auto& world = step.cosm;

		const auto character = world.create_entity(name);

		
		const auto& metas = step.input.logical_assets;
		const auto crosshair = create_character_crosshair(step);
		crosshair.get<components::crosshair>().character_entity_to_chase = character;
		crosshair.set_logic_transform(step, spawn_transform.pos);

		ingredients::add_character(metas, character, crosshair);
		
		ingredients::add_character_head_physics(step, character, spawn_transform);

		ingredients::add_character_head_inventory(step, character);


		{
			particles_existence_input effect;
			
			effect.effect.id = assets::particle_effect_id::HEALTH_DAMAGE_SPARKLES;
			effect.delete_entity_after_effect_lifetime = false;

			const auto particles = effect.create_particle_effect_entity(
				step,
				character.get_logic_transform(),
				character
			);

			particles.add_standard_components(step);
			character.get<components::sentience>().health_damage_particles = particles;
			components::particles_existence::deactivate(particles);
		}

		character.add_standard_components(step);

		// LOG("Character mass: %x", character.get<components::rigid_body>().get_mass());
		return character;
	}

	entity_handle create_character_crosshair(const logic_step step) {
		auto& world = step.cosm;
		auto root = world.create_entity("crosshair");
		auto recoil = world.create_entity("crosshair_recoil_body");
		auto zero_target = world.create_entity("zero_target");
		const auto& metas = step.input.logical_assets;

		{
			auto& sprite = root += components::sprite();
			auto& render = root += components::render();
			auto& transform = root += components::transform();
			auto& crosshair = root += components::crosshair();
			auto& processing = root += components::processing();
			
			sprite.set(assets::game_image_id::TEST_CROSSHAIR, metas, rgba(255, 255, 255, 255));

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
			components::rigid_body body;
			components::fixtures colliders;

			auto& sprite = recoil += components::sprite();

			sprite.set(assets::game_image_id::TEST_CROSSHAIR, metas, rgba(0, 255, 0, 0));

			auto& render = recoil += components::render();
			render.layer = render_layer::OVER_CROSSHAIR;
			ingredients::make_always_visible(recoil);

			body.linear_damping = 5;
			body.angular_damping = 5;

			force_joint.chased_entity = zero_target;
			//force_joint.consider_rotation = false;
			//force_joint.distance_when_force_easing_starts = 10.f;
			//force_joint.force_towards_chased_entity = 1000.f;
			//force_joint.power_of_force_easing_multiplier = 1.f;
			force_joint.divide_transform_mode = true;

			recoil += body;

			recoil.add_shape_component_from_renderable(
				step
			);

			components::fixtures group;

			group.filter = filters::trigger();
			//group.filter.categoryBits = 0;
			group.density = 0.1f;
			group.sensor = true;
			group.material = assets::physical_material_id::METAL;

			recoil += group;
			recoil.get<components::fixtures>().set_owner_body(recoil);
		}

		root.map_child_entity(child_entity_name::CROSSHAIR_RECOIL_BODY, recoil);
		
		root.add_standard_components(step);
		recoil.add_standard_components(step);
		zero_target.add_standard_components(step);

		return root;
	}
}