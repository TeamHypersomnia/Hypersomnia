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

#include "game/detail/inventory/perform_transfer.h"

namespace test_flavours {
	void populate_grenade_flavours(const populate_flavours_input in) {
		auto& flavours = in.flavours;
		auto& caches = in.caches;
		auto& plain_animations = in.plain_animations;

		(void)plain_animations;

		{
			auto& meta = get_test_flavour(flavours, test_hand_explosives::FORCE_GRENADE);

			meta.get<invariants::text_details>().description =
				"Throwable explosive with a one second delay.\nDeals damage to [color=red]Health[/color]."
			;

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);

			}
			test_flavours::add_sprite(meta, caches, test_scene_image_id::FORCE_GRENADE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("1.0");
			item.standard_price = 400;
			meta.set(item);

			{
				invariants::hand_fuse fuse; 
				fuse.release_sound.id = to_sound_id(test_scene_sound_id::GRENADE_THROW);
				fuse.armed_sound.id = to_sound_id(test_scene_sound_id::GRENADE_UNPIN);

				fuse.released_image_id = to_image_id(test_scene_image_id::FORCE_GRENADE_RELEASED);
				fuse.released_physical_material = to_physical_material_id(test_scene_physical_material_id::GRENADE);
				fuse.additional_release_impulse.linear = 2000.f;

				meta.set(fuse);
			}

			invariants::explosive explosive; 

			auto& in = explosive.explosion;
			auto& dmg = in.damage;

			in.type = adverse_element_type::FORCE;
			dmg.base = 88.f;
			in.inner_ring_color = red;
			in.outer_ring_color = orange;
			in.effective_radius = 300.f;
			dmg.impact_impulse = 550.f;
			dmg.impulse_multiplier_against_sentience = 1.f;
			in.sound.modifier.gain = 1.8f;
			in.sound.id = to_sound_id(test_scene_sound_id::GREAT_EXPLOSION);

			dmg.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
			dmg.shake.duration_ms = 500.f;
			dmg.shake.mult = 1.2f;

			meta.set(explosive);
		}

		{
			auto& meta = get_test_flavour(flavours, test_hand_explosives::INTERFERENCE_GRENADE);

			meta.get<invariants::text_details>().description =
				"Throwable explosive with a one second delay.\nDeals damage to [color=orange]Consciousness[/color].\nCauses massive aimpunch."
			;

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}
			test_flavours::add_sprite(meta, caches, test_scene_image_id::INTERFERENCE_GRENADE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.standard_price = 500;
			item.space_occupied_per_charge = to_space_units("1.0");
			meta.set(item);

			{
				invariants::hand_fuse fuse; 
				fuse.release_sound.id = to_sound_id(test_scene_sound_id::GRENADE_THROW);
				fuse.armed_sound.id = to_sound_id(test_scene_sound_id::GRENADE_UNPIN);
				fuse.released_image_id = to_image_id(test_scene_image_id::INTERFERENCE_GRENADE_RELEASED);
				fuse.released_physical_material = to_physical_material_id(test_scene_physical_material_id::GRENADE);
				fuse.additional_release_impulse.linear = 2000.f;

				meta.set(fuse);
			}

			invariants::explosive explosive; 

			auto& in = explosive.explosion;
			auto& dmg = in.damage;

			dmg.base = 100.f;
			in.inner_ring_color = yellow;
			in.outer_ring_color = orange;
			in.effective_radius = 450.f;
			dmg.impact_impulse = 2.f;
			dmg.impulse_multiplier_against_sentience = 1.f;
			in.sound.modifier.gain = 2.2f;
			in.sound.id = to_sound_id(test_scene_sound_id::INTERFERENCE_EXPLOSION);
			in.type = adverse_element_type::INTERFERENCE;

			dmg.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
			dmg.shake.duration_ms = 800.f;
			dmg.shake.mult = 1.5f;

			meta.set(explosive);
		}

		{
			auto& meta = get_test_flavour(flavours, test_hand_explosives::PED_GRENADE);

			meta.get<invariants::text_details>().description =
				"Throwable explosive with a one second delay.\nDrains [color=cyan]Personal Electricity[/color].\nIf the subject has [color=turquoise]Electric Shield[/color] enabled,\nthe effect is doubled."
			;

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}
			test_flavours::add_sprite(meta, caches, test_scene_image_id::PED_GRENADE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.standard_price = 300;
			item.space_occupied_per_charge = to_space_units("1.0");
			meta.set(item);

			{
				invariants::hand_fuse fuse; 

				fuse.release_sound.id = to_sound_id(test_scene_sound_id::GRENADE_THROW);
				fuse.armed_sound.id = to_sound_id(test_scene_sound_id::GRENADE_UNPIN);

				fuse.released_image_id = to_image_id(test_scene_image_id::PED_GRENADE_RELEASED);
				fuse.released_physical_material = to_physical_material_id(test_scene_physical_material_id::GRENADE);
				fuse.additional_release_impulse.linear = 2000.f;

				meta.set(fuse);
			}

			invariants::explosive explosive; 

			auto& in = explosive.explosion;
			auto& dmg = in.damage;

			dmg.base = 88.f;
			in.inner_ring_color = cyan;
			in.outer_ring_color = turquoise;
			in.effective_radius = 350.f;
			dmg.impact_impulse = 2.f;
			dmg.impulse_multiplier_against_sentience = 1.f;
			in.sound.modifier.gain = 2.2f;
			in.sound.id = to_sound_id(test_scene_sound_id::PED_EXPLOSION);
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
			in.effective_radius = 500.f;
			dmg.impact_impulse = 950.f;
			dmg.impulse_multiplier_against_sentience = 1.f;
			in.sound.modifier.gain = 2.f;
			in.sound.id = to_sound_id(test_scene_sound_id::BOMB_EXPLOSION);

			dmg.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
			dmg.shake.duration_ms = 700.f;
			dmg.shake.mult = 1.4f;

			return in;
		}();

		auto bomb_cascade_explosion = bomb_explosion;
		//bomb_cascade_explosion *= 0.7f;
		bomb_cascade_explosion.sound.id = to_sound_id(test_scene_sound_id::CASCADE_EXPLOSION);
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
			e *= 0.4f;
			e.sound.modifier.gain = 1.f;
			e.sound.id = to_sound_id(test_scene_sound_id::SKULL_ROCKET_DESTRUCTION);
			e.inner_ring_color = yellow;
			e.outer_ring_color = orange;
			e.ring_duration_seconds = 0.3f;
			e.wave_shake_radius_mult = 6;

			auto& meta = get_test_flavour(flavours, test_explosion_bodies::SKULL_ROCKET_CASCADE);
			auto& c = meta.get<invariants::cascade_explosion>();
			c.explosion = e;
			c.explosion_interval_ms = { 200.f, 0.4f };
			c.circle_collider_radius = 50.f;
			c.max_explosion_angle_displacement = 10.f;

			test_flavours::add_explosion_body(meta);
		}

		{
			auto e = bomb_explosion;
			e *= 0.2f;
			e.wave_shake_radius_mult = 6;
			e.sound.modifier.gain = 1.f;
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
			smaller_bomb_cascade_explosion.sound.modifier.gain = 1.f;
			smaller_bomb_cascade_explosion.sound.id = to_sound_id(test_scene_sound_id::FIREWORK);
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

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BOMB_1, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("30");
			item.categories_for_slot_compatibility = { item_category::GENERAL, item_category::BACK_WEARABLE };
			item.wear_sound.id = to_sound_id(test_scene_sound_id::BACKPACK_WEAR);

			meta.set(item);

			invariants::hand_fuse fuse; 
			fuse.release_sound.id = to_sound_id(test_scene_sound_id::GRENADE_THROW);
			fuse.armed_sound.id = to_sound_id(test_scene_sound_id::GRENADE_UNPIN);
			fuse.override_release_impulse = true;
			fuse.additional_release_impulse = {};
#if 0
			fuse.fuse_delay_ms = 35000.f;
			fuse.set_bomb_vars(1500.f, 10000.f);
#else
			fuse.fuse_delay_ms = 35000.f;
			fuse.set_bomb_vars(1500.f, 7000.f);
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
			fuse.defused_particles.modifier.colorize = cyan;
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
				dmg.base = 112.f;
				in.inner_ring_color = orange;
				in.outer_ring_color = red;
				in.effective_radius = 400.f;
				dmg.impact_impulse = 450.f;
				dmg.impulse_multiplier_against_sentience = 1.f;
				in.sound.modifier.gain = 2.f;
				in.sound.id = to_sound_id(test_scene_sound_id::SKULL_ROCKET_DESTRUCTION);
				in.sound.modifier.max_distance = 8000.f;
				in.sound.modifier.reference_distance = 1400.f;
				in.sound.modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;

				in.create_thunders_effect = true;
				in.wave_shake_radius_mult = 6;

				dmg.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
				dmg.shake.duration_ms = 700.f;
				dmg.shake.mult = 1.4f;

				explosive.explosion = in;

				{
					auto& c = explosive.cascade[0];
					c.flavour_id = to_entity_flavour_id(test_explosion_bodies::SKULL_ROCKET_CASCADE);
					c.num_spawned = 2;
					c.num_explosions = { 2, 0 };
					c.initial_speed = { 1000.f, 0.2f };
				}

				{
					auto& c = explosive.cascade[1];
					c.flavour_id = to_entity_flavour_id(test_explosion_bodies::SKULL_ROCKET_CASCADE_SMALLER);
					c.num_spawned = 3;
					c.num_explosions = { 2, 0 };
					c.initial_speed = { 1200.f, 0.6f };
					c.spawn_angle_variation = 0.5f;
				}

				meta.set(explosive);
			}
		}
	}
}
