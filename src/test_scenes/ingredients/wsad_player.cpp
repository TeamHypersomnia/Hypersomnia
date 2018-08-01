#include "test_scenes/ingredients/ingredients.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"

#include "game/assets/all_logical_assets.h"

#include "game/components/crosshair_component.h"
#include "game/components/sprite_component.h"
#include "game/components/movement_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/driver_component.h"
#include "game/components/force_joint_component.h"
#include "game/components/sentience_component.h"
#include "game/components/attitude_component.h"
#include "game/components/flags_component.h"
#include "game/components/shape_polygon_component.h"
#include "game/stateless_systems/particles_existence_system.h"
#include "test_scenes/test_scene_animations.h"

#include "game/enums/filters.h"
#include "game/enums/faction_type.h"
#include "game/detail/inventory/perform_transfer.h"

namespace test_flavours {
	void populate_character_flavours(const populate_flavours_input in) {
		auto& flavours = in.flavours;
		auto& caches = in.caches;
		auto& plain_animations = in.plain_animations;

		(void)plain_animations;

		{
			auto& meta = get_test_flavour(flavours, test_controlled_characters::METROPOLIS_SOLDIER);

			meta.get<invariants::text_details>().description = "Member of Atlantic nations.";

			{
				invariants::render render_def;
				render_def.layer = render_layer::SENTIENCES;

				meta.set(render_def);
			}

			{
				invariants::torso torso_def;

				torso_def.forward_legs = to_animation_id(test_scene_legs_animation_id::SILVER_TROUSERS);
				torso_def.strafe_legs = to_animation_id(test_scene_legs_animation_id::SILVER_TROUSERS_STRAFE);

				torso_def.stances[item_holding_stance::BARE_LIKE].carry = to_animation_id(test_scene_torso_animation_id::METROPOLIS_CHARACTER_BARE);
				torso_def.stances[item_holding_stance::BARE_LIKE].shoot = to_animation_id(test_scene_torso_animation_id::METROPOLIS_CHARACTER_BARE);

				torso_def.stances[item_holding_stance::RIFLE_LIKE].carry = to_animation_id(test_scene_torso_animation_id::METROPOLIS_CHARACTER_RIFLE);
				torso_def.stances[item_holding_stance::RIFLE_LIKE].shoot = to_animation_id(test_scene_torso_animation_id::METROPOLIS_CHARACTER_RIFLE);

				torso_def.stances[item_holding_stance::PISTOL_LIKE].carry = to_animation_id(test_scene_torso_animation_id::METROPOLIS_CHARACTER_AKIMBO);
				torso_def.stances[item_holding_stance::PISTOL_LIKE].shoot = to_animation_id(test_scene_torso_animation_id::METROPOLIS_CHARACTER_AKIMBO);

				torso_def.stances[item_holding_stance::HEAVY_LIKE].carry = to_animation_id(test_scene_torso_animation_id::METROPOLIS_CHARACTER_HEAVY);
				torso_def.stances[item_holding_stance::HEAVY_LIKE].shoot = to_animation_id(test_scene_torso_animation_id::METROPOLIS_CHARACTER_HEAVY_SHOOT);

				torso_def.stances[item_holding_stance::AKIMBO].carry = to_animation_id(test_scene_torso_animation_id::METROPOLIS_CHARACTER_AKIMBO);
				torso_def.stances[item_holding_stance::AKIMBO].shoot = to_animation_id(test_scene_torso_animation_id::METROPOLIS_CHARACTER_AKIMBO);

				meta.set(torso_def);
			}

			{
				invariants::head head_def;

				head_def.head_image = to_image_id(test_scene_image_id::METROPOLIS_HEAD);
				head_def.shooting_head_image = to_image_id(test_scene_image_id::METROPOLIS_HEAD);
				head_def.shake_rotation_damping = 10.f;
				head_def.impulse_mult_on_shake = 1000.f;

				meta.set(head_def);
			}

			add_sprite(meta, caches, test_scene_image_id::METROPOLIS_CHARACTER_BARE_1);

			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_PAST_CONTAGIOUS);
				meta.set(flags_def);
			}

			invariants::rigid_body body;
			invariants::fixtures fixtures_invariant;

			body.angled_damping = true;
			body.allow_sleep = false;

			body.damping.linear = 6.5f;
			body.damping.angular = 6.5f;

			fixtures_invariant.filter = filters::local_character();
			fixtures_invariant.density = 1.0;

			meta.set(body);
			meta.set(fixtures_invariant);

			{
				invariants::container container; 

				{
					inventory_slot slot_def;
					slot_def.physical_behaviour = slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY;
					slot_def.always_allow_exactly_one_item = true;
					slot_def.category_allowed = item_category::GENERAL;
					container.slots[slot_function::PRIMARY_HAND] = slot_def;
				}

				{
					inventory_slot slot_def;
					slot_def.physical_behaviour = slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY;
					slot_def.always_allow_exactly_one_item = true;
					slot_def.category_allowed = item_category::GENERAL;
					container.slots[slot_function::SECONDARY_HAND] = slot_def;
				}

				{
					inventory_slot slot_def;
					slot_def.category_allowed = item_category::SHOULDER_WEARABLE;
					slot_def.physical_behaviour = slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY;
					slot_def.always_allow_exactly_one_item = true;
					container.slots[slot_function::SHOULDER] = slot_def;
				}

				meta.set(container);
			}

			invariants::sentience sentience; 
			components::sentience sentience_inst;

			sentience.health_decrease_particles.id = to_particle_effect_id(test_scene_particle_effect_id::HEALTH_DAMAGE_SPARKLES);
			sentience.health_decrease_particles.modifier.colorize = red;
			sentience.health_decrease_particles.modifier.scale_lifetimes = 1.5f;

			sentience.health_decrease_sound.id = to_sound_id(test_scene_sound_id::IMPACT);
			sentience.death_sound.id = to_sound_id(test_scene_sound_id::DEATH);

			sentience.loss_of_consciousness_sound.id = to_sound_id(test_scene_sound_id::DEATH);
			sentience.consciousness_decrease_sound.id = to_sound_id(test_scene_sound_id::IMPACT);

			sentience_inst.get<health_meter_instance>().set_value(100);
			sentience_inst.get<health_meter_instance>().set_maximum_value(100);

			meta.set(sentience);
			meta.set(sentience_inst);

			invariants::movement movement;

			movement.input_acceleration_axes.set(1, 1);
			movement.acceleration_length = 4300;
			movement.braking_damping = 12.5f;
			movement.standard_linear_damping = 20.f;

			meta.set(movement);

			{
				invariants::crosshair crosshair; 
				crosshair.appearance.set(to_image_id(test_scene_image_id::TEST_CROSSHAIR), caches);

				crosshair.recoil_damping.linear = { 5, 5 };
				crosshair.recoil_damping.angular = 5;

				meta.set(crosshair);
			}

			{
				components::crosshair crosshair;
				crosshair.base_offset.set(-20, 0);
				crosshair.sensitivity.set(3, 3);
				crosshair.base_offset_bound.set(1920, 1080);
				meta.set(crosshair);
			}

			{
				components::attitude attitude;
				attitude.official_faction = faction_type::METROPOLIS;
				meta.set(attitude);
			}

			{
				invariants::item_slot_transfers transfers; 
				meta.set(transfers);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_controlled_characters::RESISTANCE_SOLDIER);
			meta = get_test_flavour(flavours, test_controlled_characters::METROPOLIS_SOLDIER);
			meta.get<invariants::text_details>().name = format_enum(test_controlled_characters::RESISTANCE_SOLDIER);

			{
				invariants::torso torso_def;

				torso_def.forward_legs = to_animation_id(test_scene_legs_animation_id::SILVER_TROUSERS);
				torso_def.strafe_legs = to_animation_id(test_scene_legs_animation_id::SILVER_TROUSERS);

				torso_def.stances[item_holding_stance::BARE_LIKE].carry = to_animation_id(test_scene_torso_animation_id::RESISTANCE_CHARACTER_BARE);
				torso_def.stances[item_holding_stance::BARE_LIKE].shoot = to_animation_id(test_scene_torso_animation_id::RESISTANCE_CHARACTER_BARE);

				torso_def.stances[item_holding_stance::RIFLE_LIKE].carry = to_animation_id(test_scene_torso_animation_id::RESISTANCE_CHARACTER_RIFLE);
				torso_def.stances[item_holding_stance::RIFLE_LIKE].shoot = to_animation_id(test_scene_torso_animation_id::RESISTANCE_CHARACTER_RIFLE_SHOOT);

				torso_def.stances[item_holding_stance::PISTOL_LIKE].carry = to_animation_id(test_scene_torso_animation_id::RESISTANCE_CHARACTER_BARE);
				torso_def.stances[item_holding_stance::PISTOL_LIKE].shoot = to_animation_id(test_scene_torso_animation_id::RESISTANCE_CHARACTER_BARE);

				torso_def.stances[item_holding_stance::HEAVY_LIKE].carry = to_animation_id(test_scene_torso_animation_id::RESISTANCE_CHARACTER_RIFLE);
				torso_def.stances[item_holding_stance::HEAVY_LIKE].shoot = to_animation_id(test_scene_torso_animation_id::RESISTANCE_CHARACTER_RIFLE_SHOOT);

				torso_def.stances[item_holding_stance::AKIMBO].carry = to_animation_id(test_scene_torso_animation_id::RESISTANCE_CHARACTER_BARE);
				torso_def.stances[item_holding_stance::AKIMBO].shoot = to_animation_id(test_scene_torso_animation_id::RESISTANCE_CHARACTER_BARE);

				meta.set(torso_def);
			}

			{
				invariants::head head_def;

				head_def.head_image = to_image_id(test_scene_image_id::RESISTANCE_HEAD);
				head_def.shooting_head_image = to_image_id(test_scene_image_id::RESISTANCE_HEAD);
				head_def.shake_rotation_damping = 10.f;
				head_def.impulse_mult_on_shake = 1000.f;

				meta.set(head_def);
			}

			{
				components::attitude attitude;
				attitude.official_faction = faction_type::RESISTANCE;
				meta.set(attitude);
			}
		}
	}
}
