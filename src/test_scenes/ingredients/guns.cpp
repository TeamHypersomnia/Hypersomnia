#include "game/cosmos/cosmos.h"
#include "game/components/gun_component.h"
#include "game/components/item_component.h"
#include "game/components/missile_component.h"
#include "game/components/sprite_component.h"
#include "game/components/trace_component.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/components/fixtures_component.h"
#include "game/components/cartridge_component.h"
#include "game/components/explosive_component.h"

#include "game/messages/start_particle_effect.h"

#include "game/stateless_systems/sound_existence_system.h"
#include "game/stateless_systems/particles_existence_system.h"

#include "game/enums/filters.h"
#include "game/enums/item_category.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"

#include "game/detail/inventory/item_slot_transfer_request.h"

#include "test_scenes/test_scene_animations.h"
#include "test_scenes/ingredients/ingredients.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"

inventory_space_type to_space_units(const std::string& s);

namespace test_flavours {
	void populate_gun_flavours(const populate_flavours_input in) {
		auto& flavours = in.flavours;
		auto& caches = in.caches;

		/* Types for bullets etc. */

		static const auto density_mult = 0.4f;
		static const auto kickback_mult = 1.5f;

		auto set_density_mult = [&](auto& meta, const auto density) {
			meta.template get<invariants::fixtures>().density = density_mult * density;
		};

		auto set_chambering_duration_ms = [&](auto& meta, const auto duration_ms) {
			meta.template get<invariants::container>().slots[slot_function::GUN_CHAMBER].mounting_duration_ms = duration_ms;
		};

		auto default_gun_props = [&](auto& meta) {
			auto& gun_def = meta.template get<invariants::gun>();

			gun_def.steam_burst_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEAM_BURST);
			gun_def.steam_burst_sound.id = to_sound_id(test_scene_sound_id::STEAM_BURST);
			gun_def.steam_burst_sound.modifier.gain = 0.7f;
			gun_def.steam_burst_perform_diff = 0.2f;
			gun_def.steam_burst_schedule_mult = 0.65f;

			gun_def.firing_engine_particles.id = to_particle_effect_id(test_scene_particle_effect_id::MUZZLE_SMOKE);

			gun_def.firing_engine_particles.modifier.scale_amounts = 0.8f;
			gun_def.trigger_pull_sound.id = to_sound_id(test_scene_sound_id::TRIGGER_PULL);
			//gun_def.firing_engine_particles.modifier.scale_lifetimes = 0.5f;
		};

		auto only_allow_mag = [&](auto& meta, const test_container_items it) {
			meta.template get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE].only_allow_flavour = ::to_entity_flavour_id(it);
		};

		auto only_allow_chamber_charge = [&](auto& meta, const test_shootable_charges it) {
			meta.template get<invariants::container>().slots[slot_function::GUN_CHAMBER].only_allow_flavour = ::to_entity_flavour_id(it);
		};

		auto make_default_gun_container = [&default_gun_props](
			auto& meta, 
			const item_holding_stance stance, 
			const float mag_mounting_duration_ms = 500.f, 
			const float /* mag_rotation */ = -90.f,
		   	const bool magazine_hidden = false,
		   	const std::string& chamber_space = "0.01",
			const bool mag_contributes_to_space = false
		){
			invariants::container container; 

			{
				inventory_slot slot_def;

				if (magazine_hidden) {
					slot_def.physical_behaviour = slot_physical_behaviour::DEACTIVATE_BODIES;
					slot_def.space_available = max_inventory_space_v;
				}
				else {
					slot_def.make_attachment_with_max_space();
				}

				slot_def.always_allow_exactly_one_item = true;
				slot_def.category_allowed = item_category::MAGAZINE;
				slot_def.mounting_duration_ms = mag_mounting_duration_ms;
				slot_def.contributes_to_space_occupied = mag_contributes_to_space;

				container.slots[slot_function::GUN_DETACHABLE_MAGAZINE] = slot_def;
			}

			{
				inventory_slot slot_def;
				slot_def.physical_behaviour = slot_physical_behaviour::DEACTIVATE_BODIES;
				slot_def.always_allow_exactly_one_item = true;
				slot_def.category_allowed = item_category::SHOT_CHARGE;
				slot_def.space_available = to_space_units(chamber_space);
				slot_def.contributes_to_space_occupied = false;

				container.slots[slot_function::GUN_CHAMBER] = slot_def;
			}

			{
				inventory_slot slot_def;
				slot_def.make_attachment_with_max_space();
				slot_def.always_allow_exactly_one_item = true;
				slot_def.category_allowed = item_category::MUZZLE_ATTACHMENT;

				container.slots[slot_function::GUN_MUZZLE] = slot_def;
			}

			meta.set(container);

			invariants::item item;

			item.space_occupied_per_charge = 
				(meta.template get<invariants::fixtures>().density / density_mult)
				* to_space_units(typesafe_sprintf("%x", real32(meta.template get<invariants::sprite>().size.area()) / 450))
			;

			item.holding_stance = stance;
			item.wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_GUN_DRAW);
			item.gratis_ammo_pieces_with_first = 3;

			const bool under_hands = stance == item_holding_stance::HEAVY_LIKE || stance == item_holding_stance::PISTOL_LIKE;
			item.draw_over_hands = !under_hands; 
			item.draw_over_hands_when_reloading = stance != item_holding_stance::HEAVY_LIKE;

			default_gun_props(meta);

			auto& mag = meta.template get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE];

			mag.start_unmounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_START_UNLOAD);

			if (stance == item_holding_stance::RIFLE_LIKE || stance == item_holding_stance::SNIPER_LIKE) {
				mag.start_mounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_RIFLE_START_LOAD);
				mag.finish_mounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_RIFLE_FINISH_LOAD);

				mag.finish_unmounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_RIFLE_FINISH_UNLOAD);
				item.flip_when_reloading = true;
			}
			else if (stance == item_holding_stance::HEAVY_LIKE) {
				mag.start_mounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_RIFLE_START_LOAD);
				mag.finish_mounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_RIFLE_FINISH_LOAD);

				mag.finish_unmounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_RIFLE_FINISH_UNLOAD);

				item.flip_when_reloading = false;
				item.gratis_ammo_pieces_with_first = 1;
			}
			else if (stance == item_holding_stance::PISTOL_LIKE) {
				mag.start_mounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_RIFLE_START_LOAD);
				mag.finish_mounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_RIFLE_FINISH_LOAD);

				mag.finish_unmounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_PISTOL_FINISH_UNLOAD);
				item.flip_when_reloading = true;
			}

			if (meta.get_name() == "Szturm") {
				mag.finish_mounting_sound.id = to_sound_id(test_scene_sound_id::SZTURM_FINISH_LOAD);
				mag.finish_unmounting_sound.id = to_sound_id(test_scene_sound_id::SZTURM_FINISH_UNLOAD);
			}

			if (meta.get_name() == "Ao44") {
				mag.finish_mounting_sound.id = to_sound_id(test_scene_sound_id::AO44_FINISH_LOAD);
			}

			if (meta.get_name() == "BullDup 2000") {
				mag.start_unmounting_sound.id = to_sound_id(test_scene_sound_id::BULLDUP2000_START_UNLOAD);
				mag.finish_unmounting_sound.id = to_sound_id(test_scene_sound_id::BULLDUP2000_FINISH_UNLOAD);
				mag.start_mounting_sound.id = to_sound_id(test_scene_sound_id::BULLDUP2000_START_LOAD);
			}

			if (meta.get_name() == "Galilea") {
				//mag.finish_unmounting_sound.id = to_sound_id(test_scene_sound_id::GALILEA_FINISH_UNLOAD);
				//mag.finish_mounting_sound.id = to_sound_id(test_scene_sound_id::GALILEA_FINISH_LOAD);
			}

			meta.set(item);
		};
	
		{
			auto make_round_remnant_flavour = [&](
				const auto flavour_id,
				const auto image_id,
				const float lifetime_mult = 0.3f
			) {
				auto& meta = get_test_flavour(flavours, flavour_id);

				{
					invariants::remnant remnant_def;
					remnant_def.lifetime_secs = 1.f * lifetime_mult;
					remnant_def.start_shrinking_when_remaining_ms = 350.f * lifetime_mult;
					remnant_def.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
					remnant_def.trace_particles.modifier.color = orange;
					meta.set(remnant_def);
				}

				test_flavours::add_sprite(meta, caches, image_id, white);
				test_flavours::add_remnant_dynamic_body(meta);
			};

			make_round_remnant_flavour(
				test_remnant_bodies::STEEL_ROUND_REMNANT_1,
				test_scene_image_id::STEEL_ROUND_REMNANT_1
			);

			make_round_remnant_flavour(
				test_remnant_bodies::STEEL_ROUND_REMNANT_2,
				test_scene_image_id::STEEL_ROUND_REMNANT_2
			);

			make_round_remnant_flavour(
				test_remnant_bodies::STEEL_ROUND_REMNANT_3,
				test_scene_image_id::STEEL_ROUND_REMNANT_3
			);

			make_round_remnant_flavour(
				test_remnant_bodies::LEWSII_ROUND_REMNANT_1,
				test_scene_image_id::STEEL_ROUND_REMNANT_1,
				0.2f
			);

			make_round_remnant_flavour(
				test_remnant_bodies::LEWSII_ROUND_REMNANT_2,
				test_scene_image_id::STEEL_ROUND_REMNANT_2,
				0.2f
			);

			make_round_remnant_flavour(
				test_remnant_bodies::LEWSII_ROUND_REMNANT_3,
				test_scene_image_id::STEEL_ROUND_REMNANT_3,
				0.2f
			);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::SKULL_ROCKET_FLYING);

			meta.template get<invariants::text_details>().name = "Skull rocket";


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SKULL_ROCKET_FLYING, white);

			{
				{
					components::trace trace_def;
					trace_def.enabled = false;
					meta.set(trace_def);
				}
			}


			test_flavours::add_bullet_round_physics(meta);
			meta.template get<invariants::rigid_body>().damping.linear = 2.8f;
			meta.template get<invariants::fixtures>().filter = filters[predefined_filter_type::FLYING_ROCKET];
			meta.template get<invariants::fixtures>().density *= 0.07f;

			invariants::missile missile;

			missile.ricochet_born_cooldown_ms = 17.f;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.spawn_exploding_ring = false;
				dest_eff.particles.modifier.color = white;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_DESTRUCTION);
			}

			missile.use_polygon_shape = true;
			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SKULL_ROCKET_TRACE);

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SKULL_ROCKET_MUZZLE_LEAVE_EXPLOSION);
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::STEEL_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_RICOCHET);

			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::STEEL_PROJECTILE_DESTRUCTION);
			missile.damage.base = 10.f;
			missile.max_lifetime_ms = 1000.f;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::SKULL_ROCKET_FLIGHT);
			missile.trace_sound.modifier.disable_velocity = true;

			missile.trace_sound_audible_to_shooter = true;

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 1.f;
			trace_modifier.max_distance = 6000.f;
			trace_modifier.reference_distance = 2000.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::STEEL_ROUND);


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::STEEL_ROUND, white);

			{
				{
					invariants::trace trace_def;
					trace_def.max_multiplier_x = {0.370f, 1.0f};
					trace_def.max_multiplier_y = {0.f, 0.09f};
					trace_def.lengthening_duration_ms = {36.f, 366.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::STEEL_ROUND_FINISHING_TRACE);
					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);

			invariants::missile missile;

			missile.ricochet_born_cooldown_ms = 17.f;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.spawn_exploding_ring = false;
				dest_eff.particles.modifier.color = white;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_TRACE);
			missile.trace_particles.modifier.color = rgba(202, 185, 89, 255);
			missile.trace_particles.modifier.scale_amounts = 3.f;
			missile.trace_particles.modifier.scale_lifetimes = 0.3f;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::FIRE_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.color = white;//{ 255, 218, 5, 255 };
			missile.damage.base = 10;
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::STEEL_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_RICOCHET);

			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::STEEL_PROJECTILE_DESTRUCTION);

			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_1));
			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_2));
			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_3));

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::STEEL_PROJECTILE_FLIGHT);

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 1.f;
			trace_modifier.max_distance = 1320.f;
			trace_modifier.reference_distance = 50.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::GALILEA_ROUND);


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::GALILEA_ROUND, white);

			{
				{
					invariants::trace trace_def;
					trace_def.max_multiplier_x = {0.370f, 1.0f};
					trace_def.max_multiplier_y = {0.f, 0.09f};
					trace_def.lengthening_duration_ms = {36.f, 366.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::GALILEA_ROUND_FINISHING_TRACE);
					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);

			invariants::missile missile;

			missile.ricochet_born_cooldown_ms = 17.f;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.spawn_exploding_ring = false;
				dest_eff.particles.modifier.color = white;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_TRACE);
			missile.trace_particles.modifier.color = rgba(202, 185, 89, 255);
			missile.trace_particles.modifier.scale_amounts = 3.f;
			missile.trace_particles.modifier.scale_lifetimes = 0.3f;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::FIRE_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.color = white;//{ 255, 218, 5, 255 };
			missile.damage.base = 10;
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::STEEL_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_RICOCHET);

			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::STEEL_PROJECTILE_DESTRUCTION);

			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_1));
			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_2));
			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_3));

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::STEEL_PROJECTILE_FLIGHT);

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 1.f;
			trace_modifier.max_distance = 1320.f;
			trace_modifier.reference_distance = 50.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::HUNTER_ROUND);


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BULLDUP2000_ROUND, white);

			{
				{
					invariants::trace trace_def;
					trace_def.max_multiplier_x = {0.370f, 1.0f};
					trace_def.max_multiplier_y = {0.f, 0.09f};
					trace_def.lengthening_duration_ms = {36.f, 366.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::HUNTER_ROUND_FINISHING_TRACE);
					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);

			invariants::missile missile;

			missile.ricochet_born_cooldown_ms = 17.f;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.spawn_exploding_ring = true;
				dest_eff.particles.modifier.color = white;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_DESTRUCTION);

				missile.damage.effects.sentience_impact = dest_eff;
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_TRACE_PRECISE);
			missile.trace_particles.modifier.color = rgba(202, 185, 89, 255);
			missile.trace_particles.modifier.scale_amounts = 8.5f;
			missile.trace_particles.modifier.scale_lifetimes = 1.2f;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::FIRE_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.color = white;//{ 255, 218, 5, 255 };
			missile.muzzle_leave_particles.modifier.scale_amounts = 1.5f;
			missile.muzzle_leave_particles.modifier.scale_lifetimes = 1.5f;
			missile.damage.base = 10;
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::STEEL_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_RICOCHET);

			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::STEEL_PROJECTILE_DESTRUCTION);

			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_1));
			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_2));
			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_3));

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::STEEL_PROJECTILE_FLIGHT);

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 1.f;
			trace_modifier.max_distance = 1620.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::BULLDUP2000_ROUND);


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BULLDUP2000_ROUND, white);

			{
				{
					invariants::trace trace_def;
					trace_def.max_multiplier_x = {0.370f, 1.0f};
					trace_def.max_multiplier_y = {0.f, 0.09f};
					trace_def.lengthening_duration_ms = {36.f, 366.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::BULLDUP2000_ROUND_FINISHING_TRACE);
					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);

			invariants::missile missile;

			missile.ricochet_born_cooldown_ms = 17.f;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.spawn_exploding_ring = false;
				dest_eff.particles.modifier.color = white;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_TRACE_PRECISE);
			missile.trace_particles.modifier.color = rgba(202, 185, 89, 255);
			missile.trace_particles.modifier.scale_amounts = 7.f;
			missile.trace_particles.modifier.scale_lifetimes = 1.1f;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::FIRE_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.color = white;//{ 255, 218, 5, 255 };
			missile.muzzle_leave_particles.modifier.scale_amounts = 1.5f;
			missile.muzzle_leave_particles.modifier.scale_lifetimes = 1.5f;
			missile.damage.base = 10;
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::STEEL_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_RICOCHET);

			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::STEEL_PROJECTILE_DESTRUCTION);

			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_1));
			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_2));
			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_3));

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::STEEL_PROJECTILE_FLIGHT);

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 1.f;
			trace_modifier.max_distance = 1620.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::LEWSII_ROUND);
			meta = get_test_flavour(flavours, test_plain_missiles::STEEL_ROUND);
			meta.get<invariants::text_details>().name = "Lews II round";

			auto& missile = meta.get<invariants::missile>();

			missile.trace_sound.id = {};

			missile.remnant_flavours.clear();
			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::LEWSII_ROUND_REMNANT_1));
			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::LEWSII_ROUND_REMNANT_2));
			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::LEWSII_ROUND_REMNANT_3));
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::ORANGE_ROUND);


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::ORANGE_ROUND, white);

			{
				{
					invariants::trace trace_def;
					trace_def.max_multiplier_x = {0.370f, 1.0f};
					trace_def.max_multiplier_y = {0.f, 0.09f};
					trace_def.lengthening_duration_ms = {36.f, 366.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::ORANGE_ROUND_FINISHING_TRACE);
					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);

			invariants::missile missile;

			missile.ricochet_born_cooldown_ms = 17.f;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.spawn_exploding_ring = false;
				dest_eff.particles.modifier.color = white;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::FURY_THROWER_ATTACK);
			missile.trace_particles.modifier.color = rgba(255, 100, 0, 255);
			missile.trace_particles.modifier.scale_amounts = 1.1f;
			missile.trace_particles.modifier.scale_lifetimes = 0.8f;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::FIRE_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.color = white;//{ 255, 218, 5, 255 };
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
			//missile.damage.shake *= 1 / 1.3f;

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::STEEL_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_RICOCHET);

			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::STEEL_PROJECTILE_DESTRUCTION);

			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_2));
			missile.damage.base = 10;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::STEEL_PROJECTILE_FLIGHT);

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 1.f;
			trace_modifier.max_distance = 700.f;
			trace_modifier.reference_distance = 50.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::DEAGLE_ROUND);


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::DEAGLE_ROUND, white);

			{
				{
					invariants::trace trace_def;
					trace_def.max_multiplier_x = {0.370f, 1.0f};
					trace_def.max_multiplier_y = {0.f, 0.09f};
					trace_def.lengthening_duration_ms = {36.f, 366.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::DEAGLE_ROUND_FINISHING_TRACE);
					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);

			invariants::missile missile;

			missile.ricochet_born_cooldown_ms = 17.f;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.spawn_exploding_ring = false;
				dest_eff.particles.modifier.color = white;
				dest_eff.particles.modifier.scale_amounts *= 1.4f;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_TRACE);
			missile.trace_particles.modifier.color = rgba(202, 185, 89, 255);
			missile.trace_particles.modifier.scale_amounts = 35.f;
			missile.trace_particles.modifier.scale_lifetimes = 0.4f;


			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::FIRE_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.color = white;//{ 255, 218, 5, 255 };
			missile.muzzle_leave_particles.modifier *= 2.f;
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
			missile.damage.impact_impulse *= 1.5f;
			//missile.damage.shake *= 1 / 1.3f;

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::STEEL_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_RICOCHET);

			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::STEEL_PROJECTILE_DESTRUCTION);

			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_2));
			missile.damage.base = 10;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::STEEL_PROJECTILE_FLIGHT);

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 1.f;
			trace_modifier.max_distance = 1400.f;
			trace_modifier.reference_distance = 50.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::PISTOL_CYAN_ROUND);


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SHOTGUN_RED_ROUND, cyan);

			{
				{
					invariants::trace trace_def;
					trace_def.max_multiplier_x = {0.370f, 1.0f};
					trace_def.max_multiplier_y = {0.f, 0.09f};
					trace_def.lengthening_duration_ms = {36.f, 366.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::CYAN_ROUND_FINISHING_TRACE);
					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);
			meta.template get<invariants::rigid_body>().damping.linear = 2.f;

			invariants::missile missile;

			missile.ricochet_born_cooldown_ms = 17.f;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.spawn_exploding_ring = false;
				dest_eff.particles.modifier.color = cyan;
				dest_eff.particles.modifier.scale_amounts = 0.2f;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::PISTOL_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);
			missile.trace_particles.modifier.color = white;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::PISTOL_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.color = cyan;
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_RICOCHET);

			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_DISCHARGE_EXPLOSION);
			missile.damage.base = 10;
			//missile.damage.shake *= 0.85f;
			missile.max_lifetime_ms = 550.f;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_PROJECTILE_FLIGHT);

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 0.6f;
			trace_modifier.max_distance = 700.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::HPSR_ROUND);


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::HPSR_ROUND, cyan);

			{
				{
					invariants::trace trace_def;

					trace_def.max_multiplier_x = {0.670f, 1.8f};
					trace_def.max_multiplier_y = {0.f, 0.09f};
					trace_def.lengthening_duration_ms = {36.f, 466.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::HPSR_ROUND_FINISHING_TRACE);

					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);

			invariants::missile missile;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.particles.modifier.color = pink;
				dest_eff.spawn_exploding_ring = true;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION);

				missile.damage.effects.sentience_impact = dest_eff;
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);
			missile.trace_particles.modifier.color = cyan;
			missile.trace_particles.modifier.scale_amounts = 1.2f;
			missile.trace_particles.modifier.scale_lifetimes = 2.0f;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::HPSR_ROUND_MUZZLE_LEAVE_EXPLOSION);
			missile.trace_particles_fly_backwards = true;
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.modifier.color = cyan;
			missile.ricochet_born_cooldown_ms = 17.f;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_PROJECTILE_FLIGHT);
			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::HPSR_ROUND_DESTRUCTION);
			missile.damage.base = 10;

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 0.6f;
			trace_modifier.max_distance = 4500.f;
			trace_modifier.reference_distance = 600.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::SHOTGUN_RED_ROUND);


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SHOTGUN_RED_ROUND, white);

			{
				{
					invariants::trace trace_def;
					trace_def.max_multiplier_x = {0.370f, 1.0f};
					trace_def.max_multiplier_y = {0.f, 0.09f};
					trace_def.lengthening_duration_ms = {36.f, 366.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::SHOTGUN_RED_ROUND_FINISHING_TRACE);
					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);
			meta.template get<invariants::rigid_body>().damping.linear = 1.5f;

			invariants::missile missile;

			missile.ricochet_born_cooldown_ms = 17.f;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.spawn_exploding_ring = false;
				dest_eff.particles.modifier.color = red;
				dest_eff.particles.modifier.scale_amounts = 0.2f;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);
			missile.trace_particles.modifier.color = red;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::FIRE_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.color = red;
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_RICOCHET);

			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_DISCHARGE_EXPLOSION);
			missile.damage.base = 10;
			missile.damage.shake *= 0.25f;
			missile.max_lifetime_ms = 450.f;

			missile.trace_sound.id = {};

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 0.6f;
			trace_modifier.max_distance = 200.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::GRADOBICIE_ROUND);


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SHOTGUN_RED_ROUND, white);

			{
				{
					invariants::trace trace_def;
					trace_def.max_multiplier_x = {0.370f, 1.0f};
					trace_def.max_multiplier_y = {0.f, 0.09f};
					trace_def.lengthening_duration_ms = {36.f, 366.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::GRADOBICIE_ROUND_FINISHING_TRACE);
					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);
			meta.template get<invariants::rigid_body>().damping.linear = 1.5f;

			invariants::missile missile;

			missile.ricochet_born_cooldown_ms = 17.f;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.spawn_exploding_ring = false;
				dest_eff.particles.modifier.color = cyan;
				dest_eff.particles.modifier.scale_amounts = 0.2f;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::ICE_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);
			missile.trace_particles.modifier.color = cyan;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.color = cyan;
			missile.muzzle_leave_particles.modifier.scale_amounts /= 3.f;
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_RICOCHET);

			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::ICE_PROJECTILE_DESTRUCTION);
			missile.damage.effects.destruction.sound.modifier.gain = 0.7f;

			missile.damage.base = 10;
			missile.damage.shake *= 0.35f;
			missile.max_lifetime_ms = 550.f;

			missile.trace_sound.id = {};

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 0.6f;
			trace_modifier.max_distance = 300.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::ZAMIEC_ROUND);


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SHOTGUN_RED_ROUND, white);

			{
				{
					invariants::trace trace_def;
					trace_def.max_multiplier_x = {0.370f, 1.0f};
					trace_def.max_multiplier_y = {0.f, 0.09f};
					trace_def.lengthening_duration_ms = {36.f, 366.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::GRADOBICIE_ROUND_FINISHING_TRACE);
					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);
			meta.template get<invariants::rigid_body>().damping.linear = 1.35f;

			invariants::missile missile;

			missile.ricochet_born_cooldown_ms = 17.f;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.spawn_exploding_ring = false;
				dest_eff.particles.modifier.color = cyan;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::ICE_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);
			missile.trace_particles.modifier.color = cyan;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.color = cyan;
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_RICOCHET);

			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::ICE_PROJECTILE_DESTRUCTION);

			missile.damage.base = 10;
			missile.max_lifetime_ms = 650.f;

			missile.trace_sound.id = {};

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 0.6f;
			trace_modifier.max_distance = 400.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		const auto pro90_round_col = rgba(255, 234, 30, 255);

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::PRO90_ROUND);


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SHOTGUN_RED_ROUND, white);

			{
				{
					invariants::trace trace_def;
					trace_def.max_multiplier_x = {0.370f, 1.0f};
					trace_def.max_multiplier_y = {0.f, 0.09f};
					trace_def.lengthening_duration_ms = {36.f, 366.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::PRO90_ROUND_FINISHING_TRACE);
					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);
			meta.template get<invariants::rigid_body>().damping.linear = 1.2f;

			invariants::missile missile;

			missile.ricochet_born_cooldown_ms = 17.f;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.spawn_exploding_ring = false;
				dest_eff.particles.modifier.color = pro90_round_col;
				dest_eff.particles.modifier.scale_amounts = 0.2f;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);
			missile.trace_particles.modifier.color = pro90_round_col;
			missile.trace_particles.modifier.scale_amounts = 0.5f;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::PISTOL_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.color = pro90_round_col;
			missile.muzzle_leave_particles.modifier.scale_amounts *= 0.5f;
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_RICOCHET);

			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_DISCHARGE_EXPLOSION);
			missile.damage.base = 10;
			missile.damage.shake *= 0.45f;
			missile.max_lifetime_ms = 600.f;

			missile.trace_sound.id = {};

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 0.6f;
			trace_modifier.max_distance = 200.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::CYAN_ROUND);


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::ROUND_TRACE, cyan);

			{
				{
					invariants::trace trace_def;

					trace_def.max_multiplier_x = {0.670f, 1.8f};
					trace_def.max_multiplier_y = {0.f, 0.09f};
					trace_def.lengthening_duration_ms = {36.f, 466.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::CYAN_ROUND_FINISHING_TRACE);

					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);

			invariants::missile missile;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.particles.modifier.color = cyan;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);
			missile.trace_particles.modifier.color = cyan;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION);
			missile.trace_particles_fly_backwards = true;
			missile.muzzle_leave_particles.modifier.color = cyan;
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.modifier.color = cyan;
			missile.ricochet_born_cooldown_ms = 17.f;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_PROJECTILE_FLIGHT);
			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_DISCHARGE_EXPLOSION);
			missile.damage.base = 10;

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 0.6f;
			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::COVERT_CYAN_ROUND);


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::ROUND_TRACE, cyan);

			{
				{
					invariants::trace trace_def;

					trace_def.max_multiplier_x = {0.670f, 1.8f};
					trace_def.max_multiplier_y = {0.f, 0.09f};
					trace_def.lengthening_duration_ms = {36.f, 466.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::CYAN_ROUND_FINISHING_TRACE);

					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);

			invariants::missile missile;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.particles.modifier.color = cyan;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);
			missile.trace_particles.modifier.color = cyan;
			missile.trace_particles.modifier.scale_amounts *= 0.5f;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::COVERT_PISTOL_MUZZLE_LEAVE_EXPLOSION);
			missile.trace_particles_fly_backwards = true;
			missile.muzzle_leave_particles.modifier.color = cyan;
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.modifier.color = cyan;
			missile.ricochet_born_cooldown_ms = 17.f;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_PROJECTILE_FLIGHT);
			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_DISCHARGE_EXPLOSION);
			missile.damage.base = 10;

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 0.6f;
			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::SZTURM_ROUND);


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::ROUND_TRACE, cyan);

			{
				{
					invariants::trace trace_def;

					trace_def.max_multiplier_x = {0.670f, 1.8f};
					trace_def.max_multiplier_y = {0.f, 0.09f};
					trace_def.lengthening_duration_ms = {36.f, 466.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::CYAN_ROUND_FINISHING_TRACE);

					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);

			invariants::missile missile;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.particles.modifier.color = cyan;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);
			missile.trace_particles.modifier.color = cyan;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SZTURM_MUZZLE_LEAVE_EXPLOSION);
			missile.trace_particles_fly_backwards = false;
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.modifier.color = cyan;
			missile.ricochet_born_cooldown_ms = 17.f;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_PROJECTILE_FLIGHT);
			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_DISCHARGE_EXPLOSION);
			missile.damage.base = 10;

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 0.6f;
			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::BLUNAZ_MISSILE);


			test_flavours::add_sprite(meta, caches, test_scene_image_id::ELECTRIC_MISSILE, cyan);
			{
				invariants::trace trace_def;

				trace_def.max_multiplier_x = {2.0f, 0.f};
				trace_def.max_multiplier_y = {0.f, 0.f};
				trace_def.lengthening_duration_ms = {300.f, 350.f};
				trace_def.additional_multiplier = vec2(1.f, 1.f);

				trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::ELECTRIC_MISSILE_FINISHING_TRACE);

				meta.set(trace_def);
			}

			test_flavours::add_bullet_round_physics(meta);
			meta.template get<invariants::rigid_body>().damping.linear = 3.5f;

			invariants::missile missile;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.particles.modifier.color = cyan;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION);
			}

			missile.use_polygon_shape = true;
			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);
			missile.trace_particles.modifier.color = cyan;
			missile.trace_particles.modifier.scale_amounts = 2.5f;
			missile.trace_particles.modifier.scale_lifetimes = 2.f;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.color = cyan;
			missile.muzzle_leave_particles.modifier.scale_amounts = 1.8f;
			missile.muzzle_leave_particles.modifier.scale_lifetimes = 1.4f;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_PROJECTILE_FLIGHT);
			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_DISCHARGE_EXPLOSION);

			missile.homing_towards_hostile_strength = 1.0f;
			missile.damage.base = 10;
			missile.damage.impact_impulse = 450.f;
			missile.damage.impulse_multiplier_against_sentience = 1.f;
			missile.ricochet_born_cooldown_ms = 17.f;
			missile.max_lifetime_ms = 200.f;
			missile.pe_damage_ratio = 0.f;

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 0.6f;
			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);

			invariants::explosive explosive; 

			standard_explosion_input in;
			auto& dmg = in.damage;

			in.type = adverse_element_type::PED;
			dmg.base = 64.f;
			in.inner_ring_color = cyan;
			in.outer_ring_color = white;
			in.effective_radius = 400.f;
			dmg.impact_impulse = 750.f;
			dmg.impulse_multiplier_against_sentience = 1.f;
			in.sound.id = to_sound_id(test_scene_sound_id::PED_EXPLOSION);
			in.sound.modifier.max_distance = 6000.f;
			in.sound.modifier.reference_distance = 2000.f;
			in.sound.modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;

			in.create_thunders_effect = true;
			in.wave_shake_radius_mult = 6;

			dmg.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
			dmg.shake.duration_ms = 200.f;
			dmg.shake.strength = 1.2f;

			explosive.explosion = in;

			{
				auto& c = explosive.cascade[0];
				c.flavour_id = to_entity_flavour_id(test_explosion_bodies::BLUNAZ_MISSILE_CASCADE);
				c.num_spawned = 2;
				c.num_explosions = { 3, 0 };
				c.initial_speed = { 1600.f, 0.2f };
				c.spawn_spread = 45.f;
			}

			{
				auto& c = explosive.cascade[1];
				c.flavour_id = to_entity_flavour_id(test_explosion_bodies::BLUNAZ_MISSILE_CASCADE_SMALLER);
				c.num_spawned = 2;
				c.num_explosions = { 3, 0 };
				c.initial_speed = { 2000.f, 0.6f };
				c.spawn_angle_variation = 0.5f;
				c.spawn_spread = 45.f;
			}

			meta.set(explosive);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::ELECTRIC_MISSILE);


			test_flavours::add_sprite(meta, caches, test_scene_image_id::ELECTRIC_MISSILE, cyan);
			{
				invariants::trace trace_def;

				trace_def.max_multiplier_x = {2.0f, 0.f};
				trace_def.max_multiplier_y = {0.f, 0.f};
				trace_def.lengthening_duration_ms = {300.f, 350.f};
				trace_def.additional_multiplier = vec2(1.f, 1.f);

				trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::ELECTRIC_MISSILE_FINISHING_TRACE);

				meta.set(trace_def);
			}

			test_flavours::add_bullet_round_physics(meta);

			invariants::missile missile;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.particles.modifier.color = cyan;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);
			missile.trace_particles.modifier.color = cyan;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.color = cyan;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_PROJECTILE_FLIGHT);
			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_DISCHARGE_EXPLOSION);

			missile.use_polygon_shape = true;
			missile.homing_towards_hostile_strength = 1.0f;
			missile.damage.base = 24;
			missile.damage.impact_impulse = 450.f;
			missile.damage.impulse_multiplier_against_sentience = 1.f;
			missile.ricochet_born_cooldown_ms = 17.f;
			missile.pe_damage_ratio = 0.4;
			missile.max_lifetime_ms = 1000.f;

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 0.6f;
			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);

			{
				/* Make it identical except explosions */
				auto& amp = get_test_flavour(flavours, test_plain_missiles::AMPLIFIER_ARM_MISSILE);
				amp = meta;

				amp.get<invariants::text_details>().name = "Amplifier arm missile";
				amp.get<invariants::missile>().damage.base = 10;
				amp.get<invariants::missile>().max_lifetime_ms = 950.f;
			}

			invariants::explosive explosive; 

			standard_explosion_input in;
			auto& dmg = in.damage;

			in.type = adverse_element_type::FORCE;
			dmg.base = 32.f;
			in.inner_ring_color = cyan;
			in.outer_ring_color = white;
			in.effective_radius = 400.f;
			dmg.impact_impulse = 750.f;
			dmg.impulse_multiplier_against_sentience = 1.f;
			in.sound.id = to_sound_id(test_scene_sound_id::SKULL_ROCKET_DESTRUCTION);
			in.sound.modifier.max_distance = 6000.f;
			in.sound.modifier.reference_distance = 2000.f;
			in.sound.modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;

			in.create_thunders_effect = true;
			in.wave_shake_radius_mult = 6;

			dmg.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
			dmg.shake.duration_ms = 700.f;
			dmg.shake.strength = 1.4f;

			explosive.explosion = in;

			{
				auto& c = explosive.cascade[0];
				c.flavour_id = to_entity_flavour_id(test_explosion_bodies::ELECTRIC_MISSILE_CASCADE);
				c.num_spawned = 2;
				c.num_explosions = { 2, 0 };
				c.initial_speed = { 1000.f, 0.2f };
			}

			{
				auto& c = explosive.cascade[1];
				c.flavour_id = to_entity_flavour_id(test_explosion_bodies::ELECTRIC_MISSILE_CASCADE_SMALLER);
				c.num_spawned = 2;
				c.num_explosions = { 2, 0 };
				c.initial_speed = { 1200.f, 0.6f };
				c.spawn_angle_variation = 0.5f;
			}

			meta.set(explosive);
		}

		{
			auto& meta = get_test_flavour(flavours, test_remnant_bodies::CYAN_SHELL);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::CYAN_SHELL, white);
			test_flavours::add_shell_dynamic_body(meta);

			invariants::remnant remnant;
			remnant.lifetime_secs = 5.f;
			remnant.start_shrinking_when_remaining_ms = 3000.f;
			meta.set(remnant);
		}

		{
			auto& meta = get_test_flavour(flavours, test_remnant_bodies::HPSR_SHELL);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::HPSR_SHELL, white);
			test_flavours::add_shell_dynamic_body(meta);

			invariants::remnant remnant;
			remnant.lifetime_secs = 10.f;
			remnant.start_shrinking_when_remaining_ms = 3000.f;
			meta.set(remnant);
		}

		{
			auto& meta = get_test_flavour(flavours, test_remnant_bodies::BULLDUP2000_SHELL);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BULLDUP2000_SHELL, white);
			test_flavours::add_shell_dynamic_body(meta);

			invariants::remnant remnant;
			remnant.lifetime_secs = 5.f;
			remnant.start_shrinking_when_remaining_ms = 3000.f;
			meta.set(remnant);
		}

		{
			auto& meta = get_test_flavour(flavours, test_remnant_bodies::HUNTER_SHELL);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BULLDUP2000_SHELL, white);
			test_flavours::add_shell_dynamic_body(meta);

			invariants::remnant remnant;
			remnant.lifetime_secs = 5.f;
			remnant.start_shrinking_when_remaining_ms = 3000.f;
			meta.set(remnant);
		}

		{
			auto& meta = get_test_flavour(flavours, test_remnant_bodies::GALILEA_SHELL);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::GALILEA_SHELL, white);
			test_flavours::add_shell_dynamic_body(meta);

			invariants::remnant remnant;
			remnant.lifetime_secs = 5.f;
			remnant.start_shrinking_when_remaining_ms = 3000.f;
			meta.set(remnant);
		}


		{
			auto& meta = get_test_flavour(flavours, test_remnant_bodies::SHOTGUN_RED_SHELL);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SHOTGUN_RED_SHELL, white);
			test_flavours::add_shell_dynamic_body(meta);

			invariants::remnant remnant;
			remnant.lifetime_secs = 10.f;
			remnant.start_shrinking_when_remaining_ms = 3000.f;
			meta.set(remnant);
		}

		{
			auto& meta = get_test_flavour(flavours, test_remnant_bodies::GRADOBICIE_SHELL);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::GRADOBICIE_SHELL, white);
			test_flavours::add_shell_dynamic_body(meta);

			invariants::remnant remnant;
			remnant.lifetime_secs = 10.f;
			remnant.start_shrinking_when_remaining_ms = 3000.f;
			meta.set(remnant);
		}

		{
			auto& meta = get_test_flavour(flavours, test_remnant_bodies::SKULL_ROCKET_SHELL);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SKULL_ROCKET_SHELL, white);
			test_flavours::add_shell_dynamic_body(meta);

			invariants::remnant remnant;
			remnant.lifetime_secs = 20.f;
			remnant.start_shrinking_when_remaining_ms = 5000.f;
			meta.set(remnant);
		}

		{
			auto& meta = get_test_flavour(flavours, test_remnant_bodies::STEEL_SHELL);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::STEEL_SHELL, white);
			test_flavours::add_shell_dynamic_body(meta);

			invariants::remnant remnant;
			remnant.lifetime_secs = 3.f;
			remnant.start_shrinking_when_remaining_ms = 1500.f;
			meta.set(remnant);
		}

		{
			auto& meta = get_test_flavour(flavours, test_remnant_bodies::LEWSII_SHELL);
			meta = get_test_flavour(flavours, test_remnant_bodies::STEEL_SHELL);
			meta.get<invariants::text_details>().name = "Lews II shell";

			auto& remnant = meta.get<invariants::remnant>();
			remnant.lifetime_secs = 0.35f;
			remnant.start_shrinking_when_remaining_ms = 100.f;
		}

		{
			auto& meta = get_test_flavour(flavours, test_remnant_bodies::ORANGE_SHELL);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::ORANGE_SHELL, white);
			test_flavours::add_shell_dynamic_body(meta);

			invariants::remnant remnant;
			remnant.lifetime_secs = 5.f;
			remnant.start_shrinking_when_remaining_ms = 3000.f;
			meta.set(remnant);
		}

		{
			auto& meta = get_test_flavour(flavours, test_remnant_bodies::DEAGLE_SHELL);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::DEAGLE_SHELL, white);
			test_flavours::add_shell_dynamic_body(meta);

			invariants::remnant remnant;
			remnant.lifetime_secs = 5.f;
			remnant.start_shrinking_when_remaining_ms = 3000.f;
			meta.set(remnant);
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::CYAN_CHARGE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::CYAN_CHARGE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;

			meta.set(item);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.color = cyan;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::CYAN_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::CYAN_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::COVERT_CYAN_CHARGE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::CYAN_CHARGE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;

			meta.set(item);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.color = cyan;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::CYAN_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::COVERT_CYAN_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::SZTURM_CHARGE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::CYAN_CHARGE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;

			meta.set(item);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.color = cyan;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::CYAN_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::SZTURM_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::PISTOL_CYAN_CHARGE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::CYAN_CHARGE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;

			meta.set(item);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.color = cyan;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::CYAN_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::PISTOL_CYAN_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::PRO90_CHARGE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::CYAN_CHARGE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;

			meta.set(item);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.color = pro90_round_col;

				//cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::CYAN_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::PRO90_ROUND);

				cartridge.num_rounds_spawned = 2;
				cartridge.rounds_spread_degrees  = 6.5f;

				meta.set(cartridge);
			}
		}

		const auto steel_color = rgba(202, 186, 89, 255);

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::STEEL_CHARGE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::STEEL_CHARGE, steel_color);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;

			meta.set(item);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.color = steel_color;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::STEEL_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::STEEL_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::HPSR_CHARGE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::STEEL_CHARGE, steel_color);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.1");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;

			meta.set(item);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.color = pink;
				cartridge.shell_trace_particles.modifier *= 2.5f;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::HPSR_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::HPSR_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::BULLDUP2000_CHARGE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::STEEL_CHARGE, steel_color);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.1");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;

			meta.set(item);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.color = steel_color;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::BULLDUP2000_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::BULLDUP2000_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::HUNTER_CHARGE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::STEEL_CHARGE, steel_color);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.1");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;

			meta.set(item);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.color = steel_color;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::HUNTER_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::HUNTER_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::GALILEA_CHARGE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::STEEL_CHARGE, steel_color);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;

			meta.set(item);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.color = steel_color;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::GALILEA_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::GALILEA_ROUND);

				meta.set(cartridge);
			}
		}


		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::LEWSII_CHARGE);
			meta = get_test_flavour(flavours, test_shootable_charges::STEEL_CHARGE);
			meta.get<invariants::text_details>().name = "Lews II charge";
			meta.get<invariants::cartridge>().round_flavour = to_entity_flavour_id(test_plain_missiles::LEWSII_ROUND);
			meta.get<invariants::cartridge>().shell_flavour = to_entity_flavour_id(test_remnant_bodies::LEWSII_SHELL);
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::SKULL_ROCKET);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SKULL_ROCKET, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("1.0");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;
			item.standard_price = static_cast<money_type>(1000);
			item.draw_over_hands_when_reloading = false;

			meta.set(item);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.color = orange;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::SKULL_ROCKET_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::SKULL_ROCKET_FLYING);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::ORANGE_CHARGE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::ORANGE_CHARGE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.1");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;

			meta.set(item);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.color = rgba(202, 186, 89, 255);

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::ORANGE_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::ORANGE_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::DEAGLE_CHARGE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::DEAGLE_CHARGE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.1");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;

			meta.set(item);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.color = white;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::DEAGLE_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::DEAGLE_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::SHOTGUN_RED_CHARGE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SHOTGUN_RED_CHARGE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.1");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;

			meta.set(item);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.color = red;
				cartridge.shell_trace_particles.modifier.scale_amounts = 1.5f;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::SHOTGUN_RED_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::SHOTGUN_RED_ROUND);

				cartridge.num_rounds_spawned = 12;
				cartridge.rounds_spread_degrees = 18.f;
				cartridge.rounds_spread_degrees_variation = 9.f;

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::ZAMIEC_CHARGE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::GRADOBICIE_CHARGE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			{
				invariants::item item;
				item.space_occupied_per_charge = to_space_units("0.01");
				item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
				item.stackable = true;

				meta.set(item);
			}

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.color = cyan;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::CYAN_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::ZAMIEC_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::CYBERSPRAY_CHARGE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::CYAN_CHARGE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			{
				invariants::item item;
				item.space_occupied_per_charge = to_space_units("0.01");
				item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
				item.stackable = true;

				meta.set(item);
			}

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.color = cyan;

				//cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::CYAN_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::PISTOL_CYAN_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::GRADOBICIE_CHARGE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::GRADOBICIE_CHARGE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			{
				invariants::item item;
				item.space_occupied_per_charge = to_space_units("0.1");
				item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
				item.stackable = true;
				item.standard_price = static_cast<money_type>(150);

				meta.set(item);
			}

			{
				components::item item;
				item.charges = 10;
				meta.set(item);
			}

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.color = cyan;
				cartridge.shell_trace_particles.modifier.scale_amounts = 1.5f;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::GRADOBICIE_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::GRADOBICIE_ROUND);

				cartridge.num_rounds_spawned = 14;
				cartridge.rounds_spread_degrees = 12.f;
				cartridge.rounds_spread_degrees_variation = 0.f;

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::BILMER2000_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BILMER2000_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.37");
			charge_deposit_def.mounting_duration_ms = 500.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::CYAN_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("0.5");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 110;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::SZTURM_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SZTURM_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.45");
			charge_deposit_def.mounting_duration_ms = 500.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::SZTURM_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("0.6");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 150;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::DATUM_GUN_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::DATUM_GUN_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.61");
			charge_deposit_def.mounting_duration_ms = 700.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::CYAN_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("0.5");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 250;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::BAKA47_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BAKA47_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.3");
			charge_deposit_def.mounting_duration_ms = 500.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::STEEL_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("0.5");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 100;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::GALILEA_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::GALILEA_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.40");
			charge_deposit_def.mounting_duration_ms = 500.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::GALILEA_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("0.4");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 100;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::KEK9_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::KEK9_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.32");
			charge_deposit_def.mounting_duration_ms = 500.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::PISTOL_CYAN_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("0.4");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 80;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::SN69_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SN69_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.37");
			charge_deposit_def.mounting_duration_ms = 500.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::PISTOL_CYAN_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("0.4");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 80;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::WARX_FQ12_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::WARX_FQ12_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("1.0");
			charge_deposit_def.mounting_duration_ms = 500.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::SHOTGUN_RED_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("0.8");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 200;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::BULLDUP2000_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BULLDUP2000_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("2.1");
			charge_deposit_def.mounting_duration_ms = 500.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::BULLDUP2000_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("0.8");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 300;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::PRO90_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::PRO90_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.50");
			charge_deposit_def.mounting_duration_ms = 300.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::PRO90_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("1.0");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 150;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::ZAMIEC_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::ZAMIEC_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			meta.get<invariants::text_details>().name =
				"Zamieć magazine"
			;

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.40");
			charge_deposit_def.mounting_duration_ms = 300.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::ZAMIEC_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("0.8");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 100;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::CYBERSPRAY_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::CYBERSPRAY_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.5");
			charge_deposit_def.mounting_duration_ms = 300.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::CYBERSPRAY_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("0.8");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 150;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::AO44_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::AO44_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("1.0");
			charge_deposit_def.mounting_duration_ms = 500.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::ORANGE_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("0.5");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 150;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::AWKA_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::HPSR_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("1.1");
			charge_deposit_def.mounting_duration_ms = 500.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::HPSR_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("1.0");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 350;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::HUNTER_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::HUNTER_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("1.1");
			charge_deposit_def.mounting_duration_ms = 500.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::HUNTER_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("1.0");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 100;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::CALICO_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::CALICO_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("1.8");
			charge_deposit_def.mounting_duration_ms = 500.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::ORANGE_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("1.0");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 100;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::DEAGLE_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::DEAGLE_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.8");
			charge_deposit_def.mounting_duration_ms = 500.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::DEAGLE_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("1.0");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 250;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::BULWARK_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BULWARK_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.13");
			charge_deposit_def.mounting_duration_ms = 500.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::STEEL_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("1.0");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 100;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::COVERT_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::COVERT_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.12");
			charge_deposit_def.mounting_duration_ms = 500.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::COVERT_CYAN_CHARGE);
			charge_deposit_def.contributes_to_space_occupied = false;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("1.0");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 200;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::LEWSII_MAGAZINE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::LEWSII_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("2.0");
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::LEWSII_CHARGE);
			charge_deposit_def.mounting_duration_ms = 300.f;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("1.0");
				item.wield_sound.id = to_sound_id(test_scene_sound_id::MAGAZINE_DRAW);
				item.standard_price = 200;
				item.draw_over_hands_when_reloading = false;
				meta.set(item);
			}

			meta.get<invariants::text_details>().name = "Lews II magazine";
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::CYAN_ROUND_FINISHING_TRACE);
			
			test_flavours::add_sprite(meta, caches, test_scene_image_id::ROUND_TRACE, cyan);

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::CYAN_ROUND).get<invariants::trace>());
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::HPSR_ROUND_FINISHING_TRACE);
			
			test_flavours::add_sprite(meta, caches, test_scene_image_id::HPSR_ROUND, cyan);

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::HPSR_ROUND).get<invariants::trace>());
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::BULLDUP2000_ROUND_FINISHING_TRACE);
			
			test_flavours::add_sprite(meta, caches, test_scene_image_id::BULLDUP2000_ROUND, white);

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::BULLDUP2000_ROUND).get<invariants::trace>());
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::HUNTER_ROUND_FINISHING_TRACE);
			
			test_flavours::add_sprite(meta, caches, test_scene_image_id::BULLDUP2000_ROUND, white);

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::HUNTER_ROUND).get<invariants::trace>());
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::GALILEA_ROUND_FINISHING_TRACE);
			
			test_flavours::add_sprite(meta, caches, test_scene_image_id::GALILEA_ROUND, white);

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::GALILEA_ROUND).get<invariants::trace>());
			}
		}


		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::DEAGLE_ROUND_FINISHING_TRACE);
			
			test_flavours::add_sprite(meta, caches, test_scene_image_id::DEAGLE_ROUND, white);

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::DEAGLE_ROUND).get<invariants::trace>());
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::STEEL_ROUND_FINISHING_TRACE);
			
			test_flavours::add_sprite(meta, caches, test_scene_image_id::STEEL_ROUND, white);

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::STEEL_ROUND).get<invariants::trace>());
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::ORANGE_ROUND_FINISHING_TRACE);
			
			test_flavours::add_sprite(meta, caches, test_scene_image_id::ORANGE_ROUND, white);

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::ORANGE_ROUND).get<invariants::trace>());
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::SHOTGUN_RED_ROUND_FINISHING_TRACE);
			
			test_flavours::add_sprite(meta, caches, test_scene_image_id::SHOTGUN_RED_ROUND, white);

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::SHOTGUN_RED_ROUND).get<invariants::trace>());
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::GRADOBICIE_ROUND_FINISHING_TRACE);
			
			test_flavours::add_sprite(meta, caches, test_scene_image_id::SHOTGUN_RED_ROUND, white);

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::GRADOBICIE_ROUND).get<invariants::trace>());
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::PRO90_ROUND_FINISHING_TRACE);
			
			test_flavours::add_sprite(meta, caches, test_scene_image_id::SHOTGUN_RED_ROUND, white);

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::SHOTGUN_RED_ROUND).get<invariants::trace>());
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::ELECTRIC_MISSILE_FINISHING_TRACE);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::ELECTRIC_MISSILE, cyan);

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::ELECTRIC_MISSILE).get<invariants::trace>());
			}
		}


		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::BILMER2000);

			meta.get<invariants::text_details>().description =
				"Standard issue silenced rifle."
			;

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::BILMER2000_MUZZLE);
			gun_def.muzzle_shot_sound.modifier.reference_distance = 300.f;
			gun_def.muzzle_shot_sound.modifier.max_distance = 500.f;

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4500.f, 4500.f};
			gun_def.shot_cooldown_ms = 90.f;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 3.3f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 8;
			gun_def.low_ammo_cue_sound.modifier.reference_distance = 300.f;
			gun_def.low_ammo_cue_sound.modifier.max_distance = 500.f;

			gun_def.kickback_towards_wielder = kickback_mult * 8.f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(500);

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.052f;
			gun_def.recoil_multiplier = 0.5;

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::BILMER_CHAMBERING);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BILMER2000, white);
			test_flavours::add_lying_item_dynamic_body(meta).density = 0.1f;
			set_density_mult(meta, 1.35);
			make_default_gun_container(meta, item_holding_stance::RIFLE_LIKE, 1200.f);
			meta.get<invariants::item>().standard_price = 3000;
			set_chambering_duration_ms(meta, 450.f);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			only_allow_mag(meta, test_container_items::BILMER2000_MAGAZINE);
			meta.get<invariants::item>().specific_to = faction_type::METROPOLIS;
			meta.template get<invariants::item>().space_occupied_per_charge = to_space_units("6.4");
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::SZTURM);

			meta.get<invariants::text_details>().description =
				"Standard issue assault carbine."
			;

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::SZTURM_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4500.f, 4500.f};
			gun_def.shot_cooldown_ms = 84.f;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 2500.f};
			gun_def.damage_multiplier = 3.3f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 10;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.kickback_towards_wielder = kickback_mult * 22.f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(350);

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.052f;
			gun_def.recoil_multiplier = 0.55;

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::BILMER_CHAMBERING);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SZTURM, white);
			test_flavours::add_lying_item_dynamic_body(meta).density = 0.1f;
			set_density_mult(meta, 1.4);
			make_default_gun_container(meta, item_holding_stance::RIFLE_LIKE, 1100.f);
			meta.get<invariants::item>().standard_price = 3400;
			set_chambering_duration_ms(meta, 450.f);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			only_allow_mag(meta, test_container_items::SZTURM_MAGAZINE);
			meta.get<invariants::item>().specific_to = faction_type::METROPOLIS;
			meta.template get<invariants::item>().space_occupied_per_charge = to_space_units("6.0");
			meta.get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE].draw_under_container = true;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::PRO90);

			meta.get<invariants::text_details>().description =
				"Pretty good SMG."
			;

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::PRO90_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4900.f, 5400.f};
			gun_def.shot_cooldown_ms = 61.f;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 1.1;
			gun_def.head_radius_multiplier = 0.4f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 10;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.kickback_towards_wielder = kickback_mult * 1.f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(250);
			gun_def.recoil_multiplier = 0.1f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.052f;

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::PRO90_CHAMBERING);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::PRO90, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 1.15f);
			make_default_gun_container(meta, item_holding_stance::PISTOL_LIKE, 1500.f, 0.f, true);
			meta.get<invariants::item>().standard_price = 2800;
			set_chambering_duration_ms(meta, 600.f);

			auto& item = meta.get<invariants::item>();
			item.wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_SMG_DRAW);
			only_allow_mag(meta, test_container_items::PRO90_MAGAZINE);
			item.flip_when_reloading = false;
			item.draw_mag_over_when_reloading = false;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::ZAMIEC);

			meta.get<invariants::text_details>().description =
				"Pretty good SMG."
			;

			meta.get<invariants::text_details>().name =
				"Zamieć"
			;

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::ZAMIEC_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {5300.f, 6000.f};
			gun_def.shot_cooldown_ms = 75.f;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 2.4;
			gun_def.headshot_multiplier = 2.5f;
			gun_def.head_radius_multiplier = 0.85f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 7;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.kickback_towards_wielder = kickback_mult * 2.f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(600);
			gun_def.recoil_multiplier = 0.17f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.052f;

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::ZAMIEC_CHAMBERING);
			gun_def.shoot_animation = to_animation_id(test_scene_plain_animation_id::ZAMIEC_SHOT);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::ZAMIEC, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 0.9f);
			make_default_gun_container(meta, item_holding_stance::PISTOL_LIKE, 1200.f, 0.f, false);
			meta.get<invariants::item>().standard_price = 2400;
			set_chambering_duration_ms(meta, 600.f);

			auto& item = meta.get<invariants::item>();
			item.wield_sound.id = to_sound_id(test_scene_sound_id::ZAMIEC_DRAW);
			only_allow_mag(meta, test_container_items::ZAMIEC_MAGAZINE);
			item.flip_when_reloading = true;
			item.draw_mag_over_when_reloading = false;
			meta.get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE].draw_under_container = true;

			auto& mag = meta.template get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE];
			mag.finish_unmounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_RIFLE_FINISH_UNLOAD);
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::CYBERSPRAY);

			meta.get<invariants::text_details>().description =
				"Pretty good SMG."
			;

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::CYBERSPRAY_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {5550.f, 6300.f};
			gun_def.shot_cooldown_ms = 40.f;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 1.2;
			gun_def.head_radius_multiplier = 0.4f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 7;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.kickback_towards_wielder = kickback_mult * 1.f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(450);
			gun_def.recoil_multiplier = 0.04;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.042f;

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::CYBERSPRAY_CHAMBERING);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::CYBERSPRAY, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 1.2f);
			make_default_gun_container(meta, item_holding_stance::PISTOL_LIKE, 1000.f, 0.f, false);
			meta.get<invariants::item>().standard_price = 2100;
			set_chambering_duration_ms(meta, 300.f);

			auto& item = meta.get<invariants::item>();
			item.wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_PISTOL_DRAW);
			only_allow_mag(meta, test_container_items::CYBERSPRAY_MAGAZINE);
			item.flip_when_reloading = true;
			item.draw_mag_over_when_reloading = false;
			//meta.get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE].draw_under_container = true;

			auto& mag = meta.template get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE];
			mag.finish_unmounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_RIFLE_FINISH_UNLOAD);

			item.gratis_ammo_pieces_with_first = 4;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::BAKA47);

			meta.get<invariants::text_details>().description =
				"Standard issue sample rifle."
			;

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::BAKA47_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4500.f, 4500.f};
			gun_def.shot_cooldown_ms = 100.f;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 4.f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 6;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.kickback_towards_wielder = kickback_mult * 70.f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.062f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.recoil_multiplier = 0.97f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(400);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.shoot_animation = to_animation_id(test_scene_plain_animation_id::BAKA47_SHOT);
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::RIFLE_CHAMBERING);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BAKA47, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 1.4);
			make_default_gun_container(meta, item_holding_stance::RIFLE_LIKE, 1400.f, 0.f);
			meta.get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE].draw_under_container = true;
			meta.get<invariants::item>().standard_price = 2900;
			set_chambering_duration_ms(meta, 700.f);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			only_allow_mag(meta, test_container_items::BAKA47_MAGAZINE);
			meta.get<invariants::item>().specific_to = faction_type::RESISTANCE;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::GALILEA);

			meta.get<invariants::text_details>().description =
				"Standard issue sample rifle."
			;

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::GALILEA_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4400.f, 4400.f};
			gun_def.shot_cooldown_ms = 94.f;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 3.3f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 6;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.kickback_towards_wielder = kickback_mult * 80.f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.062f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.recoil_multiplier = 0.94f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(750);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::RIFLE_CHAMBERING);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::GALILEA, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 2.f);
			make_default_gun_container(meta, item_holding_stance::RIFLE_LIKE, 1750.f, 0.f);
			meta.get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE].draw_under_container = true;
			meta.get<invariants::item>().standard_price = 2100;
			set_chambering_duration_ms(meta, 700.f);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			only_allow_mag(meta, test_container_items::GALILEA_MAGAZINE);
			meta.template get<invariants::item>().space_occupied_per_charge = to_space_units("6");
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::LEWS);

			meta.get<invariants::text_details>().description =
				"Standard issue sample rifle."
			;

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::LEWSII_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4400.f, 4400.f};
			gun_def.shot_cooldown_ms = 60.f;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 3.3f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 15;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.kickback_towards_wielder = kickback_mult * 80.f;

			gun_def.heavy_heat_start_sound.id = to_sound_id(test_scene_sound_id::HEAVY_HEAT_START);
			gun_def.light_heat_start_sound.id = to_sound_id(test_scene_sound_id::LIGHT_HEAT_START);

			gun_def.steam_burst_schedule_mult = 1.f;
			gun_def.heat_cooldown_speed_mult = 4.f;

			gun_def.minimum_heat_to_shoot = 2.0f;
			gun_def.maximum_heat = 4.0f;
			gun_def.gunshot_adds_heat = 0.2f;
			gun_def.firing_engine_sound.modifier.pitch = 0.4f;
			gun_def.recoil_multiplier = 0.5f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(150);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			//gun_def.shoot_animation = to_animation_id(test_scene_plain_animation_id::BAKA47_SHOT);
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::RIFLE_CHAMBERING);

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::LEWSII, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 1.25f);
			make_default_gun_container(meta, item_holding_stance::HEAVY_LIKE, 2000.f, 0.f, false, "0.01", true);

			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::LEWSII_DRAW);
			meta.get<invariants::item>().standard_price = 5100;
			set_chambering_duration_ms(meta, 700.f);

			only_allow_mag(meta, test_container_items::LEWSII_MAGAZINE);
			meta.get<invariants::text_details>().name = "Lews II";
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::DATUM);

			meta.get<invariants::text_details>().description =
				"Standard issue sample rifle."
			;

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::PLASMA_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {5100.f, 5100.f};
			gun_def.shot_cooldown_ms = 95.f;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 3.3f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 10;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.kickback_towards_wielder = kickback_mult * 85.f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.072f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.recoil_multiplier = 0.65f;

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.shoot_animation = to_animation_id(test_scene_plain_animation_id::DATUM_GUN_SHOT);
			gun_def.adversarial.knockout_award = static_cast<money_type>(350);
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_CHAMBERING);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::DATUM_GUN, white);

			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 1.45f);

			make_default_gun_container(meta, item_holding_stance::RIFLE_LIKE, 1500.f, 0.f, true);
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::PLASMA_DRAW);
			meta.get<invariants::item>().standard_price = 3900;
			set_chambering_duration_ms(meta, 900.f);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			only_allow_mag(meta, test_container_items::DATUM_GUN_MAGAZINE);
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::KEK9);

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::KEK9_MUZZLE);

			gun_def.action_mode = gun_action_type::SEMI_AUTOMATIC;
			gun_def.muzzle_velocity = {4900.f, 4900.f};
			gun_def.shot_cooldown_ms = 100.f;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 2.f;
			gun_def.headshot_multiplier = 2.5f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 8;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.042f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.recoil_multiplier = 0.5f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::MEDIUM_PISTOL_CHAMBERING);
			gun_def.adversarial.knockout_award = static_cast<money_type>(850);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::KEK9, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 0.8f);
			make_default_gun_container(meta, item_holding_stance::PISTOL_LIKE, 1000.f, 0.f, false);
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_PISTOL_DRAW);
			meta.get<invariants::item>().standard_price = 350;
			set_chambering_duration_ms(meta, 390.f);
			meta.get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE].draw_under_container = true;
			only_allow_mag(meta, test_container_items::KEK9_MAGAZINE);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			meta.get<invariants::item>().specific_to = faction_type::RESISTANCE;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::SN69);

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::SN69_MUZZLE);

			gun_def.action_mode = gun_action_type::SEMI_AUTOMATIC;
			gun_def.muzzle_velocity = {4900.f, 4900.f};
			gun_def.shot_cooldown_ms = 90.f;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 12.f;
			gun_def.shell_velocity = {300.f, 1900.f};
			gun_def.damage_multiplier = 1.8f;
			gun_def.headshot_multiplier = 2.5f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 9;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.recoil_multiplier = 0.4f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(900);

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.040;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::LIGHT_PISTOL_CHAMBERING);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SN69, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 0.7f);
			make_default_gun_container(meta, item_holding_stance::PISTOL_LIKE, 1000.f, 0.f, true);
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_PISTOL_DRAW);
			meta.get<invariants::item>().standard_price = 350;
			set_chambering_duration_ms(meta, 350.f);
			only_allow_mag(meta, test_container_items::SN69_MAGAZINE);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			meta.get<invariants::item>().specific_to = faction_type::METROPOLIS;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::AWKA);

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::HPSR_MUZZLE);
			gun_def.muzzle_shot_sound.modifier.max_distance = 8000.f;
			gun_def.muzzle_shot_sound.modifier.reference_distance = 2000.f;

			gun_def.action_mode = gun_action_type::BOLT_ACTION;
			gun_def.muzzle_velocity = {11000.f, 11000.f};
			gun_def.shot_cooldown_ms = 600.f;
			gun_def.after_transfer_shot_cooldown_mult = 0.5f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::HPSR_CHAMBERING);
			gun_def.allow_chambering_with_akimbo = false;

			gun_def.delay_shell_spawn_until_chambering = true;
			gun_def.shell_spawn_delay_mult = 0.15f;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 12.f;
			gun_def.shell_velocity = {2500.f, 3500.f};
			gun_def.damage_multiplier = 19.8f;
			gun_def.headshot_multiplier = 3.0f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 2;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.recoil_multiplier = 4.5f;
			gun_def.kickback_towards_wielder = kickback_mult * 220.f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.202f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.adversarial.knockout_award = static_cast<money_type>(250);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.recoil.pattern_progress_per_shot *= 9.0f;

			test_flavours::add_sprite(meta, caches, test_scene_image_id::HPSR, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 2.0f);
			make_default_gun_container(meta, item_holding_stance::SNIPER_LIKE, 1850.f, 0.f, false, "0.1");
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_SNIPER_DRAW);
			meta.get<invariants::item>().standard_price = 4900;
			set_chambering_duration_ms(meta, 650.f);
			meta.template get<invariants::item>().space_occupied_per_charge = to_space_units("8");
			only_allow_mag(meta, test_container_items::AWKA_MAGAZINE);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			meta.get<invariants::item>().flip_when_reloading = true;
			meta.get<invariants::item>().gratis_ammo_pieces_with_first = 2;
			meta.get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE].draw_under_container = true;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::HUNTER);

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::HUNTER_MUZZLE);
			gun_def.muzzle_shot_sound.modifier.max_distance = 8000.f;
			gun_def.muzzle_shot_sound.modifier.reference_distance = 2000.f;

			gun_def.action_mode = gun_action_type::BOLT_ACTION;
			gun_def.muzzle_velocity = {10000.f, 10000.f};
			gun_def.shot_cooldown_ms = 400.f;
			gun_def.after_transfer_shot_cooldown_mult = 0.5f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::HUNTER_CHAMBERING);
			gun_def.allow_chambering_with_akimbo = false;

			gun_def.delay_shell_spawn_until_chambering = true;
			gun_def.shell_spawn_delay_mult = 0.15f;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 12.f;
			gun_def.shell_velocity = {2500.f, 3500.f};
			gun_def.damage_multiplier = 9.6f;
			gun_def.headshot_multiplier = 3.0f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 2;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.recoil_multiplier = 2.8f;
			gun_def.kickback_towards_wielder = kickback_mult * 130.f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.202f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.adversarial.knockout_award = static_cast<money_type>(800);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.recoil.pattern_progress_per_shot *= 6.0f;

			test_flavours::add_sprite(meta, caches, test_scene_image_id::HUNTER, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 0.8f);
			make_default_gun_container(meta, item_holding_stance::SNIPER_LIKE, 1500.f, 0.f, false, "0.1");
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_SNIPER_DRAW);
			meta.get<invariants::item>().standard_price = 2400;
			set_chambering_duration_ms(meta, 590.f);
			meta.template get<invariants::item>().space_occupied_per_charge = to_space_units("6");
			only_allow_mag(meta, test_container_items::HUNTER_MAGAZINE);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			meta.get<invariants::item>().flip_when_reloading = true;
			meta.get<invariants::item>().gratis_ammo_pieces_with_first = 2;
			meta.get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE].draw_under_container = true;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::AO44);

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::AO44_MUZZLE);

			gun_def.delay_shell_spawn_until_chambering = true;
			gun_def.shell_spawn_delay_mult = 0.15f;

			gun_def.action_mode = gun_action_type::BOLT_ACTION;
			gun_def.muzzle_velocity = {4500.f, 4500.f};
			gun_def.shot_cooldown_ms = 100.f;
			gun_def.after_transfer_shot_cooldown_mult = 1.5f;
			gun_def.burst_interval_ms = 45.f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::REVOLVER_CHAMBERING);
			gun_def.allow_chambering_with_akimbo = true;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 12.f;
			gun_def.shell_velocity = {500.f, 2500.f};
			gun_def.damage_multiplier = 6.f;
			gun_def.headshot_multiplier = 3.0f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 3;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.recoil_multiplier = 2.f;
			gun_def.kickback_towards_wielder = kickback_mult * 120.f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.202f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.adversarial.knockout_award = static_cast<money_type>(700);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.recoil.pattern_progress_per_shot *= 4.0f;

			gun_def.burst_spread_degrees = 11.f;
			gun_def.burst_spread_degrees_variation = 0.f;
			gun_def.after_burst_chambering_ms = 1000.f;
			gun_def.num_burst_bullets = 3;
			gun_def.burst_recoil_mult = 0.4f;

			test_flavours::add_sprite(meta, caches, test_scene_image_id::AO44, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 2.3f);
			make_default_gun_container(meta, item_holding_stance::PISTOL_LIKE, 1500.f, 0.f, false, "0.1");
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_PISTOL_DRAW);
			meta.get<invariants::item>().standard_price = 1000;
			set_chambering_duration_ms(meta, 200.f);
			meta.template get<invariants::item>().space_occupied_per_charge = to_space_units("6");
			only_allow_mag(meta, test_container_items::AO44_MAGAZINE);
			meta.get<invariants::item>().draw_mag_over_when_reloading = true;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::ELON);

			meta.template get<invariants::text_details>().name = "Rocket Launcher ELON";

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::ELON_HRL_MUZZLE);
			gun_def.muzzle_shot_sound.modifier.max_distance = 6000.f;
			gun_def.muzzle_shot_sound.modifier.reference_distance = 2000.f;

			gun_def.action_mode = gun_action_type::BOLT_ACTION;
			gun_def.muzzle_velocity = {5200.f, 5200.f};
			gun_def.shot_cooldown_ms = 350.f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::AUTOMATIC_SHOTGUN_CHAMBERING);
			gun_def.allow_charge_in_chamber_magazine_when_chamber_loaded = false;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 12.f;
			gun_def.shell_velocity = {500.f, 2500.f};
			gun_def.damage_multiplier = 11.2f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 0;
			gun_def.recoil_multiplier = 4.f;
			gun_def.kickback_towards_wielder = kickback_mult * 500.f;

			gun_def.minimum_heat_to_shoot = 0.0f;

			gun_def.heavy_heat_start_sound.id = to_sound_id(test_scene_sound_id::ELON_HRL_LEVER);
			gun_def.light_heat_start_sound.id = to_sound_id(test_scene_sound_id::ELON_HRL_LEVER);

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.202f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.adversarial.knockout_award = static_cast<money_type>(100);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.recoil.pattern_progress_per_shot *= 12.0f;
			gun_def.allow_chambering_with_akimbo = true;

			test_flavours::add_sprite(meta, caches, test_scene_image_id::ELON_HRL, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 1.9f);
			make_default_gun_container(meta, item_holding_stance::HEAVY_LIKE, 0.f, 0.f, false, "1.0");

			auto& slots = meta.get<invariants::container>().slots;

			only_allow_chamber_charge(meta, test_shootable_charges::SKULL_ROCKET);

			slots.erase(slot_function::GUN_DETACHABLE_MAGAZINE);

			slots[slot_function::GUN_CHAMBER].physical_behaviour = slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY;
			slots[slot_function::GUN_CHAMBER].never_reachable_for_mounting = true;

			{
				auto& mag = slots[slot_function::GUN_CHAMBER_MAGAZINE];
				mag = slots[slot_function::GUN_CHAMBER];
				mag.never_reachable_for_mounting = false;
				mag.mounting_duration_ms = 2900.f;
				mag.start_mounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_RIFLE_START_LOAD);
				mag.finish_mounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_RIFLE_FINISH_LOAD);
			}

			set_chambering_duration_ms(meta, 600.f);

			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_PISTOL_DRAW);
			meta.get<invariants::item>().standard_price = 8000;
			meta.template get<invariants::item>().space_occupied_per_charge = to_space_units("13.0");

			meta.get<invariants::item>().draw_mag_over_when_reloading = true;
			meta.get<invariants::item>().gratis_ammo_pieces_with_first = 2;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::BULLDUP2000);

			meta.template get<invariants::text_details>().name = "BullDup 2000";

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::BULLDUP2000_MUZZLE);
			gun_def.muzzle_shot_sound.modifier.max_distance = 5500.f;
			gun_def.muzzle_shot_sound.modifier.reference_distance = 1800.f;

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = { 8000.0f, 8000.0f };
			gun_def.shot_cooldown_ms = 230.f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::BULLDUP2000_CHAMBERING);
			gun_def.allow_chambering_with_akimbo = false;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 12.f;
			gun_def.shell_velocity = {900.f, 3500.f};
			gun_def.damage_multiplier = 8.4f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 5;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.recoil_multiplier = 2.f;
			gun_def.kickback_towards_wielder = kickback_mult * 60.f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.222f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.adversarial.knockout_award = static_cast<money_type>(100);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BULLDUP2000, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 2.0f);
			make_default_gun_container(meta, item_holding_stance::RIFLE_LIKE, 1400.f, 0.f, false, "0.1");
			meta.get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE].draw_under_container = true;
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_GUN_DRAW);
			meta.get<invariants::item>().standard_price = 5000;
			set_chambering_duration_ms(meta, 700.f);
			meta.template get<invariants::item>().space_occupied_per_charge = to_space_units("9");
			only_allow_mag(meta, test_container_items::BULLDUP2000_MAGAZINE);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			meta.get<invariants::item>().flip_when_reloading = true;
			meta.get<invariants::item>().gratis_ammo_pieces_with_first = 2;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::WARX);

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::WARX_FQ12_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = { 4600.f, 5600.f };
			gun_def.shot_cooldown_ms = 200.f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::AUTOMATIC_SHOTGUN_CHAMBERING);
			gun_def.allow_chambering_with_akimbo = false;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 12.f;
			gun_def.shell_velocity = {900.f, 3500.f};
			gun_def.damage_multiplier = 0.8f;
			gun_def.head_radius_multiplier = 0.4f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 4;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.recoil_multiplier = 1.0f;
			gun_def.kickback_towards_wielder = kickback_mult * 60.f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.242f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.adversarial.knockout_award = static_cast<money_type>(150);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.recoil.pattern_progress_per_shot *= 3.0f;

			test_flavours::add_sprite(meta, caches, test_scene_image_id::WARX_FQ12, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 1.5f);
			make_default_gun_container(meta, item_holding_stance::RIFLE_LIKE, 1400.f, 0.f, false, "0.1");
			meta.get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE].draw_under_container = true;
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_GUN_DRAW);
			meta.get<invariants::item>().standard_price = 2900;
			set_chambering_duration_ms(meta, 650.f);
			meta.template get<invariants::item>().space_occupied_per_charge = to_space_units("6");
			only_allow_mag(meta, test_container_items::WARX_FQ12_MAGAZINE);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			meta.get<invariants::item>().flip_when_reloading = true;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::GRADOBICIE);

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::GRADOBICIE_MUZZLE);

			gun_def.action_mode = gun_action_type::BOLT_ACTION;
			gun_def.muzzle_velocity = { 4200.f, 6300.f };
			gun_def.shot_cooldown_ms = 300.f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::GRADOBICIE_CHAMBERING);
			gun_def.allow_chambering_with_akimbo = false;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 13.f;
			gun_def.shell_velocity = {900.f, 3500.f};
			gun_def.damage_multiplier = 1.2f;
			gun_def.head_radius_multiplier = 0.4f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 5;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.recoil_multiplier = 2.9f;
			gun_def.kickback_towards_wielder = kickback_mult * 150.f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.442f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.adversarial.knockout_award = static_cast<money_type>(700);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.recoil.pattern_progress_per_shot *= 5.0f;
			
			test_flavours::add_sprite(meta, caches, test_scene_image_id::GRADOBICIE, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 0.9f);
			make_default_gun_container(meta, item_holding_stance::RIFLE_LIKE, 1400.f, 0.f, false, "0.1");
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_GUN_DRAW);
			meta.get<invariants::item>().standard_price = 2100;
			set_chambering_duration_ms(meta, 550.f);
			meta.template get<invariants::item>().space_occupied_per_charge = to_space_units("6.5");
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			meta.get<invariants::item>().flip_when_reloading = true;
			meta.get<invariants::item>().gratis_ammo_pieces_with_first = 2;

			auto& slots = meta.get<invariants::container>().slots;

			only_allow_chamber_charge(meta, test_shootable_charges::GRADOBICIE_CHARGE);

			slots.erase(slot_function::GUN_DETACHABLE_MAGAZINE);

			slots[slot_function::GUN_CHAMBER].physical_behaviour = slot_physical_behaviour::DEACTIVATE_BODIES;
			slots[slot_function::GUN_CHAMBER].never_reachable_for_mounting = true;

			{
				auto& mag = slots[slot_function::GUN_CHAMBER_MAGAZINE];
				mag = slots[slot_function::GUN_CHAMBER];
				mag.never_reachable_for_mounting = false;
				mag.mounting_duration_ms = 500.f;
				mag.start_mounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_START_UNLOAD);
				mag.finish_mounting_sound.id = to_sound_id(test_scene_sound_id::SHOTGUN_INSERT_CHARGE);
				mag.contributes_to_space_occupied = false;
				mag.space_available = to_space_units("0.9");
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::CALICO);

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::CALICO_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4150.f, 4150.f};
			gun_def.shot_cooldown_ms = 110.f;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 12.f;
			gun_def.shell_velocity = {300.f, 1900.f};
			gun_def.damage_multiplier = 4.f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 6;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.recoil_multiplier = 1.02f;
			gun_def.kickback_towards_wielder = kickback_mult * 30.f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(600);

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.072f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.shoot_animation = to_animation_id(test_scene_plain_animation_id::CALICO_SHOT);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::CALICO_CHAMBERING);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::CALICO, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 1.5f);
			make_default_gun_container(meta, item_holding_stance::PISTOL_LIKE, 1500.f, 0.f, false, "0.1");
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_PISTOL_DRAW);
			meta.get<invariants::item>().standard_price = 700;
			set_chambering_duration_ms(meta, 500.f);
			only_allow_mag(meta, test_container_items::CALICO_MAGAZINE);
			meta.get<invariants::item>().draw_mag_over_when_reloading = true;
			meta.get<invariants::item>().specific_to = faction_type::SPECTATOR;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::BULWARK);

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::BULWARK_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4150.f, 4150.f};
			gun_def.shot_cooldown_ms = 140.f;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 12.f;
			gun_def.shell_velocity = {300.f, 1900.f};
			gun_def.damage_multiplier = 4.7f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 4;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.recoil_multiplier = 1.25f;
			gun_def.kickback_towards_wielder = kickback_mult * 50.f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(700);

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.092f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.shoot_animation = to_animation_id(test_scene_plain_animation_id::BULWARK_SHOT);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::HEAVY_PISTOL_CHAMBERING);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BULWARK, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 1.85f);
			make_default_gun_container(meta, item_holding_stance::PISTOL_LIKE, 1400.f, 0.f, false, "0.01");
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_PISTOL_DRAW);
			meta.get<invariants::item>().standard_price = 500;
			set_chambering_duration_ms(meta, 450.f);
			only_allow_mag(meta, test_container_items::BULWARK_MAGAZINE);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			meta.get<invariants::item>().specific_to = faction_type::SPECTATOR;
			meta.get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE].draw_under_container = true;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::COVERT);

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::COVERT_MUZZLE);
			gun_def.muzzle_shot_sound.modifier.reference_distance = 200.f;
			gun_def.muzzle_shot_sound.modifier.max_distance = 400.f;

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4600.f, 4600.f};
			gun_def.shot_cooldown_ms = 150.f;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 12.f;
			gun_def.shell_velocity = {300.f, 1900.f};
			gun_def.damage_multiplier = 3.5f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 3;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.low_ammo_cue_sound.modifier.reference_distance = 300.f;
			gun_def.low_ammo_cue_sound.modifier.max_distance = 500.f;
			gun_def.recoil_multiplier = 0.91f;
			gun_def.kickback_towards_wielder = kickback_mult * 20.f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(700);

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.092f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.shoot_animation = to_animation_id(test_scene_plain_animation_id::COVERT_SHOT);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::HEAVY_PISTOL_CHAMBERING);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::COVERT, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 1.5f);
			make_default_gun_container(meta, item_holding_stance::PISTOL_LIKE, 1400.f, 0.f, false, "0.01");
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_PISTOL_DRAW);
			meta.get<invariants::item>().standard_price = 1100;
			set_chambering_duration_ms(meta, 500.f);
			only_allow_mag(meta, test_container_items::COVERT_MAGAZINE);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			meta.get<invariants::item>().specific_to = faction_type::SPECTATOR;
			meta.get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE].draw_under_container = true;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::DEAGLE);

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::DEAGLE_MUZZLE);
			gun_def.muzzle_shot_sound.modifier.max_distance = 7000.f;
			gun_def.muzzle_shot_sound.modifier.reference_distance = 1500.f;

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4650.f, 4650.f};
			gun_def.shot_cooldown_ms = 500.f;

			gun_def.shell_angular_velocity = {10000.f, 40000.f};
			gun_def.shell_spread_degrees = 12.f;
			gun_def.shell_velocity = {500.f, 3200.f};
			gun_def.damage_multiplier = 9.2f;
			gun_def.headshot_multiplier = 3.0f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 2;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.recoil_multiplier = 2.75f;
			gun_def.kickback_towards_wielder = kickback_mult * 170.f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(900);

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.35f;
			gun_def.heat_cooldown_speed_mult = 2.f;
			gun_def.firing_engine_sound.modifier.pitch = 0.4f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.shoot_animation = to_animation_id(test_scene_plain_animation_id::DEAGLE_SHOT);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.recoil.pattern_progress_per_shot *= 2.5f;

			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::DEAGLE_CHAMBERING);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::DEAGLE, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 2.6f);
			make_default_gun_container(meta, item_holding_stance::PISTOL_LIKE, 1500.f, 0.f, false, "0.1");
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_PISTOL_DRAW);
			meta.get<invariants::item>().standard_price = 1200;
			set_chambering_duration_ms(meta, 500.f);
			only_allow_mag(meta, test_container_items::DEAGLE_MAGAZINE);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			meta.get<invariants::item>().specific_to = faction_type::SPECTATOR;
			meta.get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE].draw_under_container = true;
			meta.template get<invariants::item>().space_occupied_per_charge = to_space_units("6");
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::AMPLIFIER_ARM);

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::ASSAULT_RIFLE_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {1800.f, 1800.f};
			gun_def.shot_cooldown_ms = 500.f;
			gun_def.headshot_multiplier = 2.0f;
			gun_def.head_radius_multiplier = 0.2f;

			gun_def.damage_multiplier = 3.2f;

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.magic_missile_flavour = to_entity_flavour_id(test_plain_missiles::AMPLIFIER_ARM_MISSILE);
			gun_def.adversarial.knockout_award = static_cast<money_type>(150);

			default_gun_props(meta);
			set_density_mult(meta, 0.8f);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::AMPLIFIER_ARM, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("3.0");
			item.holding_stance = item_holding_stance::RIFLE_LIKE;
			item.gratis_ammo_pieces_with_first = 0;
			meta.set(item);
			meta.get<invariants::item>().standard_price = 4100;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::BLUNAZ);

			auto& gun_def = meta.get<invariants::gun>();

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::BLUNAZ_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {5000.f, 5000.f};
			gun_def.shot_cooldown_ms = 1800.f;
			gun_def.after_transfer_shot_cooldown_mult = 0.4f;

			gun_def.damage_multiplier = 1.f;

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.magic_missile_flavour = to_entity_flavour_id(test_plain_missiles::BLUNAZ_MISSILE);
			gun_def.adversarial.knockout_award = static_cast<money_type>(1350);
			gun_def.shoot_animation = to_animation_id(test_scene_plain_animation_id::BLUNAZ_SHOT);
			gun_def.kickback_towards_wielder = kickback_mult * 150.f;
			gun_def.recoil_multiplier = 3.25f;

			default_gun_props(meta);
			set_density_mult(meta, 0.7f);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BLUNAZ, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("5.0");
			item.holding_stance = item_holding_stance::RIFLE_LIKE;
			item.gratis_ammo_pieces_with_first = 0;
			meta.set(item);
			meta.get<invariants::item>().standard_price = 2600;
		}
	}
}

float get_penetration(const test_shootable_weapons w) {
	using W = test_shootable_weapons;

	switch(w) {
		case W::AMPLIFIER_ARM:  return 0.0f;
		case W::ELON:           return 0.0f;
		case W::BLUNAZ:         return 0.0f;
		case W::KEK9:           return 15.0f;
		case W::SN69:           return 15.0f;
		case W::CALICO:         return 30.0f;
		case W::BULWARK:        return 30.0f;
		case W::COVERT:         return 40.0f;
		case W::PRO90:          return 40.0f;
		case W::WARX:           return 40.0f;
		case W::GRADOBICIE:     return 40.0f;
		case W::CYBERSPRAY:     return 50.0f;
		case W::ZAMIEC:         return 50.0f;
		case W::GALILEA:        return 110.0f;
		case W::BILMER2000:     return 110.0f;
		case W::BAKA47:         return 110.0f;
		case W::SZTURM:         return 110.0f;
		case W::DATUM:          return 110.0f;
		case W::LEWS:           return 110.0f;
		case W::AO44:           return 110.0f;
		case W::BULLDUP2000:    return 120.0f;
		case W::DEAGLE:         return 130.0f;
		case W::HUNTER:         return 140.0f;
		case W::AWKA:           return 150.0f;
		default: return 0.0f;
	}
}
