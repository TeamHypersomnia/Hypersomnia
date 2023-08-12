#include "test_scenes/ingredients/ingredients.h"
#include "test_scenes/test_scene_sounds.h"
#include "test_scenes/test_scene_animations.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

#include "game/assets/ids/asset_ids.h"

#include "game/components/missile_component.h"
#include "game/components/item_component.h"
#include "game/components/melee_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/explosive_component.h"
#include "game/components/shape_circle_component.h"
#include "game/components/shape_polygon_component.h"
#include "game/components/sender_component.h"
#include "game/components/hand_fuse_component.h"

#include "game/enums/filters.h"

inventory_space_type to_space_units(const std::string& s);

namespace test_flavours {
	void populate_grenade_flavours(const populate_flavours_input in) {
		auto& flavours = in.flavours;
		auto& caches = in.caches;
		auto& plain_animations = in.plain_animations;

		(void)plain_animations;

		{
			auto& meta = get_test_flavour(flavours, test_hand_explosives::FORCE_GRENADE);

			meta.get<invariants::text_details>().description =
				"Deals damage to Health."
			;
			test_flavours::add_sprite(meta, caches, test_scene_image_id::FORCE_GRENADE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("1.0");
			item.standard_price = 1000;
			meta.set(item);

			{
				invariants::hand_fuse fuse; 
				fuse.release_sound.id = to_sound_id(test_scene_sound_id::GRENADE_THROW);
				fuse.armed_sound.id = to_sound_id(test_scene_sound_id::GRENADE_UNPIN);

				fuse.released_image_id = to_image_id(test_scene_image_id::FORCE_GRENADE_RELEASED);
				fuse.released_physical_material = to_physical_material_id(test_scene_physical_material_id::GRENADE);
				fuse.additional_release_impulse.linear = 3000.f;
				fuse.additional_secondary_release_impulse.linear = 2000.f;

				meta.set(fuse);
			}

			{
				components::hand_fuse fuse;
				fuse.fuse_delay_ms = 1200.f;
				meta.set(fuse);
			}

			invariants::explosive explosive; 

			auto& in = explosive.explosion;
			auto& dmg = in.damage;

			in.type = adverse_element_type::FORCE;

			{
				auto& c = explosive.cascade[0];
				c.flavour_id = to_entity_flavour_id(test_explosion_bodies::FORCE_GRENADE_CASCADE);
				c.num_spawned = 2;
				c.num_explosions = { 2, 0 };
				c.initial_speed = { 1300.f, 0.2f };
			}

			dmg.base = 88.f;
			in.inner_ring_color = red;
			in.outer_ring_color = orange;
			in.effective_radius = 380.f;
			dmg.impact_impulse = 550.f;
			dmg.impulse_multiplier_against_sentience = 1.f;
			in.sound.id = to_sound_id(test_scene_sound_id::GREAT_EXPLOSION);
			in.sound.modifier.max_distance = 6000.f;
			in.sound.modifier.reference_distance = 2000.f;

			dmg.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
			dmg.shake.duration_ms = 1900.f;
			dmg.shake.strength = 1.2f;

			meta.set(explosive);
		}

		{
			standard_explosion_input e;
			auto& dmg = e.damage;

			e.type = adverse_element_type::FORCE;
			dmg.base = 60.f;
			e.effective_radius = 600.f * 0.4f;
			dmg.impact_impulse = 950.f;
			dmg.impulse_multiplier_against_sentience = 1.f;
			e.sound.modifier.max_distance = 6000.f;
			e.sound.modifier.reference_distance = 2000.f;
			dmg.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
			dmg.shake.duration_ms = 1500.f;
			dmg.shake.strength = 1.4f;
			e.sound.id = to_sound_id(test_scene_sound_id::SKULL_ROCKET_DESTRUCTION);
			e.inner_ring_color = yellow;
			e.outer_ring_color = orange;
			e.ring_duration_seconds = 0.3f;
			e.wave_shake_radius_mult = 6;

			auto& meta = get_test_flavour(flavours, test_explosion_bodies::FORCE_GRENADE_CASCADE);
			auto& c = meta.get<invariants::cascade_explosion>();
			c.explosion = e;
			c.explosion_interval_ms = { 240.f, 0.2f };
			c.circle_collider_radius = 50.f;
			c.max_explosion_angle_displacement = 10.f;

			test_flavours::add_explosion_body(meta);
		}
		{
			auto& meta = get_test_flavour(flavours, test_hand_explosives::INTERFERENCE_GRENADE);

			meta.get<invariants::text_details>().description =
				"Kicks enemies far away. Also drains stamina."
			;
			test_flavours::add_sprite(meta, caches, test_scene_image_id::INTERFERENCE_GRENADE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.standard_price = 550;
			item.space_occupied_per_charge = to_space_units("1.0");
			meta.set(item);

			{
				invariants::hand_fuse fuse; 
				fuse.release_sound.id = to_sound_id(test_scene_sound_id::GRENADE_THROW);
				fuse.armed_sound.id = to_sound_id(test_scene_sound_id::GRENADE_UNPIN);
				fuse.released_image_id = to_image_id(test_scene_image_id::INTERFERENCE_GRENADE_RELEASED);
				fuse.released_physical_material = to_physical_material_id(test_scene_physical_material_id::GRENADE);
				fuse.additional_release_impulse.linear = 3000.f;
				fuse.additional_secondary_release_impulse.linear = 2000.f;

				meta.set(fuse);
			}

			{
				components::hand_fuse fuse;
				fuse.fuse_delay_ms = 800.f;
				meta.set(fuse);
			}

			invariants::explosive explosive; 
			explosive.adversarial.knockout_award = 0;

			auto& in = explosive.explosion;
			auto& dmg = in.damage;

			dmg.base = 150.f;
			in.inner_ring_color = yellow;
			in.outer_ring_color = orange;
			in.effective_radius = 450.f;
			dmg.impact_impulse = 2.f;
			dmg.impulse_multiplier_against_sentience = 3000.f;
			in.sound.id = to_sound_id(test_scene_sound_id::INTERFERENCE_EXPLOSION);
			in.sound.modifier.max_distance = 6000.f;
			in.sound.modifier.reference_distance = 2000.f;
			in.type = adverse_element_type::INTERFERENCE;

			dmg.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
			dmg.shake.duration_ms = 2000.f;
			dmg.shake.strength = 1.6f;

			{
				auto e = in;
				e *= 0.6f;
				e.damage.impact_impulse = 2.f * 0.6f;
				e.wave_shake_radius_mult = 6;
				e.sound.modifier.gain = 1.f;
				e.sound.id = to_sound_id(test_scene_sound_id::INTERFERENCE_EXPLOSION);
				e.sound.modifier.gain = 0.1f;
				e.sound.modifier.pitch = 0.75f;
				e.sound.modifier.max_distance = 6000.f;
				e.sound.modifier.reference_distance = 2000.f;
				e.inner_ring_color = orange;
				e.outer_ring_color = yellow;
				e.ring_duration_seconds = 0.3f;

				auto& meta = get_test_flavour(flavours, test_explosion_bodies::INTERFERENCE_CASCADE);
				auto& c = meta.get<invariants::cascade_explosion>();
				c.explosion = e;
				c.explosion_interval_ms = { 200.f, 0.4f };
				c.circle_collider_radius = 50.f;
				c.max_explosion_angle_displacement = 10.f;

				test_flavours::add_explosion_body(meta);
			}

			{
				auto& c = explosive.cascade[0];
				c.flavour_id = to_entity_flavour_id(test_explosion_bodies::INTERFERENCE_CASCADE);
				c.num_spawned = 3;
				c.num_explosions = { 4, 0 };
				c.initial_speed = { 2000.f, 0.3f };
			}

			meta.set(explosive);
		}

		{
			auto& meta = get_test_flavour(flavours, test_hand_explosives::FLASHBANG);

			meta.get<invariants::text_details>().description =
				"Blinds and deafens the enemy for a short while."
			;
			test_flavours::add_sprite(meta, caches, test_scene_image_id::FLASHBANG, white);
			auto& fixtures = test_flavours::add_lying_item_dynamic_body(meta);
			fixtures.restitution = 1.f;

			invariants::item item;
			item.standard_price = 700;
			item.space_occupied_per_charge = to_space_units("1.0");
			meta.set(item);

			{
				invariants::hand_fuse fuse; 
				fuse.release_sound.id = to_sound_id(test_scene_sound_id::GRENADE_THROW);
				fuse.armed_sound.id = to_sound_id(test_scene_sound_id::GRENADE_UNPIN);
				fuse.released_image_id = to_image_id(test_scene_image_id::FLASHBANG_RELEASED);
				fuse.released_physical_material = to_physical_material_id(test_scene_physical_material_id::FLASHBANG);
				fuse.additional_release_impulse.linear = 3000.f;
				fuse.additional_secondary_release_impulse.linear = 2000.f;

				meta.set(fuse);
			}

			{
				components::hand_fuse fuse;
				fuse.fuse_delay_ms = 800.f;
				meta.set(fuse);
			}

			invariants::explosive explosive; 
			explosive.adversarial.knockout_award = 0;

			auto& in = explosive.explosion;
			auto& dmg = in.damage;

			dmg.base = 6.f;
			in.inner_ring_color = white;
			in.outer_ring_color = white;
			in.effective_radius = 1250.f;
			dmg.impact_impulse = 0.f;
			dmg.impulse_multiplier_against_sentience = 0.f;
			in.sound.id = to_sound_id(test_scene_sound_id::FLASHBANG_EXPLOSION);
			in.sound.modifier.max_distance = 6000.f;
			in.sound.modifier.reference_distance = 2000.f;
			in.type = adverse_element_type::FLASH;

			dmg.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
			dmg.shake.duration_ms = 1400.f;
			dmg.shake.strength = 0.9f;

			meta.set(explosive);
		}

		{
			auto& meta = get_test_flavour(flavours, test_hand_explosives::PED_GRENADE);

			meta.get<invariants::text_details>().description =
				"Drains Personal Electricity and destroys armor."
			;
			test_flavours::add_sprite(meta, caches, test_scene_image_id::PED_GRENADE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.standard_price = 800;
			item.space_occupied_per_charge = to_space_units("1.0");
			meta.set(item);

			{
				invariants::hand_fuse fuse; 

				fuse.release_sound.id = to_sound_id(test_scene_sound_id::GRENADE_THROW);
				fuse.armed_sound.id = to_sound_id(test_scene_sound_id::GRENADE_UNPIN);

				fuse.released_image_id = to_image_id(test_scene_image_id::PED_GRENADE_RELEASED);
				fuse.released_physical_material = to_physical_material_id(test_scene_physical_material_id::GRENADE);
				fuse.additional_release_impulse.linear = 3000.f;
				fuse.additional_secondary_release_impulse.linear = 2000.f;

				meta.set(fuse);
			}

			{
				components::hand_fuse fuse;
				fuse.fuse_delay_ms = 800.f;
				meta.set(fuse);
			}

			invariants::explosive explosive; 
			explosive.adversarial.knockout_award = 0;

			auto& in = explosive.explosion;
			auto& dmg = in.damage;

			dmg.base = 45.0f;
			in.inner_ring_color = cyan;
			in.outer_ring_color = turquoise;
			in.effective_radius = 700.f;
			dmg.impact_impulse = 2.f;
			dmg.impulse_multiplier_against_sentience = 1.f;
			in.sound.id = to_sound_id(test_scene_sound_id::PED_EXPLOSION);
			in.sound.modifier.max_distance = 6000.f;
			in.sound.modifier.reference_distance = 2000.f;
			in.type = adverse_element_type::PED;
			in.create_thunders_effect = true;

			dmg.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
			dmg.shake = { 0.f, 0.f };

			meta.set(explosive);
		}

		const auto bomb_explosion = []() {
			standard_explosion_input in;
			auto& dmg = in.damage;

			in.type = adverse_element_type::FORCE;
			dmg.base = 348.f;
			in.inner_ring_color = cyan;
			in.outer_ring_color = white;
			in.effective_radius = 600.f;
			dmg.impact_impulse = 950.f;
			dmg.impulse_multiplier_against_sentience = 1.f;
			in.sound.id = to_sound_id(test_scene_sound_id::BOMB_EXPLOSION);
			in.sound.modifier.max_distance = 6000.f;
			in.sound.modifier.reference_distance = 2000.f;

			dmg.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
			dmg.shake.duration_ms = 2500.f;
			dmg.shake.strength = 1.4f;

			return in;
		}();

		auto bomb_cascade_explosion = bomb_explosion;
		//bomb_cascade_explosion *= 0.7f;
		bomb_cascade_explosion.sound.id = to_sound_id(test_scene_sound_id::SKULL_ROCKET_DESTRUCTION);
		bomb_cascade_explosion.inner_ring_color = white;
		bomb_cascade_explosion.outer_ring_color = cyan;
		bomb_cascade_explosion.ring_duration_seconds = 0.3f;

		{
			auto& meta = get_test_flavour(flavours, test_explosion_bodies::BOMB_CASCADE_EXPLOSION);
			auto& c = meta.get<invariants::cascade_explosion>();
			c.explosion = bomb_cascade_explosion;
			c.explosion_interval_ms = { 200.f, 0.4f };
			c.circle_collider_radius = 50.f;
			c.max_explosion_angle_displacement = 10.f;

			test_flavours::add_explosion_body(meta);
		}

		{
			auto e = bomb_explosion;
			e *= 0.3f;
			e.sound.id = to_sound_id(test_scene_sound_id::SKULL_ROCKET_DESTRUCTION);
			e.inner_ring_color = cyan;
			e.outer_ring_color = white;
			e.ring_duration_seconds = 0.3f;
			e.wave_shake_radius_mult = 6;

			auto& meta = get_test_flavour(flavours, test_explosion_bodies::ELECTRIC_MISSILE_CASCADE);
			auto& c = meta.get<invariants::cascade_explosion>();
			c.explosion = e;
			c.explosion_interval_ms = { 200.f, 0.4f };
			c.circle_collider_radius = 50.f;
			c.max_explosion_angle_displacement = 10.f;

			test_flavours::add_explosion_body(meta);
		}

		{
			auto e = bomb_explosion;
			e *= 0.1f;
			e.wave_shake_radius_mult = 6;
			e.sound.id = to_sound_id(test_scene_sound_id::GREAT_EXPLOSION);
			e.inner_ring_color = white;
			e.outer_ring_color = cyan;
			e.ring_duration_seconds = 0.3f;

			auto& meta = get_test_flavour(flavours, test_explosion_bodies::ELECTRIC_MISSILE_CASCADE_SMALLER);
			auto& c = meta.get<invariants::cascade_explosion>();
			c.explosion = e;
			c.explosion_interval_ms = { 200.f, 0.4f };
			c.circle_collider_radius = 50.f;
			c.max_explosion_angle_displacement = 10.f;

			test_flavours::add_explosion_body(meta);
		}

		{
			auto e = bomb_explosion;
			e *= 0.3f;
			e.sound.id = to_sound_id(test_scene_sound_id::PED_EXPLOSION);
			e.inner_ring_color = cyan;
			e.outer_ring_color = turquoise;
			e.ring_duration_seconds = 0.3f;
			e.wave_shake_radius_mult = 6;
			e.type = adverse_element_type::PED;

			auto& meta = get_test_flavour(flavours, test_explosion_bodies::BLUNAZ_MISSILE_CASCADE);
			auto& c = meta.get<invariants::cascade_explosion>();
			c.explosion = e;
			c.explosion_interval_ms = { 240.f, 0.5f };
			c.circle_collider_radius = 50.f;
			c.max_explosion_angle_displacement = 10.f;

			test_flavours::add_explosion_body(meta);
		}

		{
			auto e = bomb_explosion;
			e *= 0.2f;
			e.wave_shake_radius_mult = 6;
			e.sound.id = to_sound_id(test_scene_sound_id::PED_EXPLOSION);
			e.inner_ring_color = turquoise;
			e.outer_ring_color = cyan;
			e.ring_duration_seconds = 0.3f;
			e.type = adverse_element_type::PED;

			auto& meta = get_test_flavour(flavours, test_explosion_bodies::BLUNAZ_MISSILE_CASCADE_SMALLER);
			auto& c = meta.get<invariants::cascade_explosion>();
			c.explosion = e;
			c.explosion_interval_ms = { 220.f, 0.5f };
			c.circle_collider_radius = 50.f;
			c.max_explosion_angle_displacement = 10.f;

			test_flavours::add_explosion_body(meta);
		}

		{
			auto e = bomb_explosion;
			e *= 0.4f;
			e.sound.id = to_sound_id(test_scene_sound_id::SKULL_ROCKET_DESTRUCTION);
			e.inner_ring_color = yellow;
			e.outer_ring_color = orange;
			e.ring_duration_seconds = 0.3f;
			e.wave_shake_radius_mult = 6;

			auto& meta = get_test_flavour(flavours, test_explosion_bodies::SKULL_ROCKET_CASCADE);
			auto& c = meta.get<invariants::cascade_explosion>();
			c.explosion = e;
			c.explosion_interval_ms = { 220.f, 0.4f };
			c.circle_collider_radius = 50.f;
			c.max_explosion_angle_displacement = 10.f;

			test_flavours::add_explosion_body(meta);
		}

		{
			auto e = bomb_explosion;
			e *= 0.35f;
			e.wave_shake_radius_mult = 6;
			e.sound.id = to_sound_id(test_scene_sound_id::GREAT_EXPLOSION);
			e.inner_ring_color = red;
			e.outer_ring_color = orange;
			e.ring_duration_seconds = 0.3f;

			auto& meta = get_test_flavour(flavours, test_explosion_bodies::SKULL_ROCKET_CASCADE_SMALLER);
			auto& c = meta.get<invariants::cascade_explosion>();
			c.explosion = e;
			c.explosion_interval_ms = { 200.f, 0.4f };
			c.circle_collider_radius = 50.f;
			c.max_explosion_angle_displacement = 10.f;

			test_flavours::add_explosion_body(meta);
		}

		{
			auto smaller_bomb_cascade_explosion = bomb_explosion;
			smaller_bomb_cascade_explosion *= 0.07f;
			smaller_bomb_cascade_explosion.sound.id = {};
			smaller_bomb_cascade_explosion.inner_ring_color = green;
			smaller_bomb_cascade_explosion.outer_ring_color = dark_green;
			smaller_bomb_cascade_explosion.ring_duration_seconds = 0.3f;

			auto& meta = get_test_flavour(flavours, test_explosion_bodies::BOMB_CASCADE_EXPLOSION_SMALLER);
			auto& c = meta.get<invariants::cascade_explosion>();
			c.explosion = smaller_bomb_cascade_explosion;
			c.explosion_interval_ms = { 60.f, 0.5f };
			c.circle_collider_radius = 5.f;
			c.max_explosion_angle_displacement = 30.f;

			test_flavours::add_explosion_body(meta);
		}

		{
			auto& meta = get_test_flavour(flavours, test_hand_explosives::BOMB);

			meta.get<invariants::text_details>().description =
				"Can be planted. Deals massive damage nearby."
			;

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BOMB_1, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("30");
			item.categories_for_slot_compatibility = { item_category::GENERAL, item_category::OVER_BACK_WEARABLE };
			item.wear_sound.id = to_sound_id(test_scene_sound_id::BACKPACK_WEAR);

			meta.set(item);

			{
				components::hand_fuse fuse;
				fuse.fuse_delay_ms = 35000.f;
				meta.set(fuse);
			}

			invariants::hand_fuse fuse; 
			fuse.release_sound.id = to_sound_id(test_scene_sound_id::GRENADE_THROW);
			fuse.armed_sound.id = to_sound_id(test_scene_sound_id::GRENADE_UNPIN);
			fuse.override_release_impulse = true;
			fuse.additional_release_impulse = {};
			fuse.additional_secondary_release_impulse = {};
#if 0
			fuse.fuse_delay_ms = 35000.f;
			fuse.set_bomb_vars(1500.f, 10000.f);
#else
			fuse.set_bomb_vars(2500.f, 8000.f);
#endif
			fuse.beep_sound.id = to_sound_id(test_scene_sound_id::BEEP);
			fuse.beep_sound.modifier.doppler_factor = 0.5f;
			fuse.beep_color = red;
			fuse.beep_time_mult = 0.08f;
			fuse.started_arming_sound.id = to_sound_id(test_scene_sound_id::BOMB_PLANTING);
			fuse.started_defusing_sound.id = to_sound_id(test_scene_sound_id::STARTED_DEFUSING);
			fuse.defused_sound[0].id = to_sound_id(test_scene_sound_id::STEAM_BURST);
			fuse.defused_sound[1].id = to_sound_id(test_scene_sound_id::POWER_OUT);
			fuse.defused_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEAM_BURST);
			fuse.defused_particles.modifier.color = cyan;
			fuse.defused_particles.modifier.scale_amounts = 2.5f;
			fuse.defused_particles.modifier.scale_lifetimes = 2.5f;

			fuse.armed_animation_id = to_animation_id(test_scene_plain_animation_id::BOMB_ARMED);
			fuse.defused_image_id = to_image_id(test_scene_image_id::BOMB_DEFUSED);

			meta.set(fuse);

			{
				invariants::explosive explosive; 
				explosive.explosion = bomb_explosion;

				{
					auto& c = explosive.cascade[0];
					c.flavour_id = to_entity_flavour_id(test_explosion_bodies::BOMB_CASCADE_EXPLOSION);
					c.num_spawned = 4;
					c.num_explosions = { 2, 1 };
					c.initial_speed = { 2000.f, 0.2f };
				}

				{
					auto& c = explosive.cascade[1];
					c.flavour_id = to_entity_flavour_id(test_explosion_bodies::BOMB_CASCADE_EXPLOSION_SMALLER);
					c.num_spawned = 7;
					c.num_explosions = { 7, 2 };
					c.initial_speed = { 2200.f, 0.6f };
					c.spawn_angle_variation = 0.5f;
				}

				meta.set(explosive);
			}
			invariants::animation anim;
			anim.id = to_animation_id(test_scene_plain_animation_id::BOMB);
			meta.set(anim);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::SKULL_ROCKET_FLYING);

			{
				invariants::explosive explosive; 

				standard_explosion_input in;
				auto& dmg = in.damage;

				in.type = adverse_element_type::FORCE;
				dmg.base = 142.f;
				in.inner_ring_color = orange;
				in.outer_ring_color = red;
				in.effective_radius = 500.f;
				dmg.impact_impulse = 350.f;
				dmg.impulse_multiplier_against_sentience = 1.f;
				in.sound.id = to_sound_id(test_scene_sound_id::SKULL_ROCKET_DESTRUCTION);
				in.sound.modifier.max_distance = 6000.f;
				in.sound.modifier.reference_distance = 2000.f;
				in.sound.modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;

				in.create_thunders_effect = true;
				in.wave_shake_radius_mult = 6;

				dmg.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
				dmg.shake.duration_ms = 2100.f;
				dmg.shake.strength = 1.4f;

				explosive.explosion = in;

				{
					auto& c = explosive.cascade[0];
					c.flavour_id = to_entity_flavour_id(test_explosion_bodies::SKULL_ROCKET_CASCADE);
					c.num_spawned = 3;
					c.num_explosions = { 2, 0 };
					c.initial_speed = { 1300.f, 0.2f };
				}

				{
					auto& c = explosive.cascade[1];
					c.flavour_id = to_entity_flavour_id(test_explosion_bodies::SKULL_ROCKET_CASCADE_SMALLER);
					c.num_spawned = 3;
					c.num_explosions = { 3, 0 };
					c.initial_speed = { 1200.f, 0.6f };
					c.spawn_angle_variation = 0.5f;
				}

				meta.set(explosive);
			}
		}
	}
}
