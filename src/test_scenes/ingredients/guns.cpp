#include "test_scenes/create_test_scene_entity.h"
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
#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"

namespace test_flavours {
	void populate_gun_flavours(const populate_flavours_input in) {
		auto& flavours = in.flavours;
		auto& caches = in.caches;

		/* Types for bullets etc. */

		auto set_density_mult = [&](auto& meta, const auto density) {
			meta.template get<invariants::fixtures>().density = .25f * density;
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
			item.space_occupied_per_charge = to_space_units(typesafe_sprintf("%x", real32(meta.template get<invariants::sprite>().size.area()) / 450));
			item.holding_stance = stance;
			item.wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_GUN_DRAW);
			item.gratis_ammo_pieces_with_first = 3;

			default_gun_props(meta);

			auto& mag = meta.template get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE];

			mag.start_unmounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_START_UNLOAD);

			if (stance == item_holding_stance::RIFLE_LIKE) {
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

			meta.set(item);
		};
	
		{
			auto make_round_remnant_flavour = [&](
				const auto flavour_id,
				const auto image_id
			) {
				auto& meta = get_test_flavour(flavours, flavour_id);

				{
					invariants::render render_def;
					render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

					meta.set(render_def);
				}

				{
					invariants::remnant remnant_def;
					remnant_def.lifetime_secs = 1.f;
					remnant_def.start_shrinking_when_remaining_ms = 350.f;
					remnant_def.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
					remnant_def.trace_particles.modifier.colorize = orange;
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
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::SKULL_ROCKET_FLYING);

			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
			}


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
			meta.template get<invariants::rigid_body>().damping.linear = 3.f;
			meta.template get<invariants::fixtures>().filter = filters[predefined_filter_type::FLYING_COLLIDING_BULLET];

			invariants::missile missile;

			missile.ricochet_born_cooldown_ms = 17.f;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.spawn_exploding_ring = false;
				dest_eff.particles.modifier.colorize = white;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SKULL_ROCKET_TRACE);

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SKULL_ROCKET_MUZZLE_LEAVE_EXPLOSION);
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::STEEL_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_RICOCHET);

			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::STEEL_PROJECTILE_DESTRUCTION);
			missile.damage.base = 112.f;
			missile.max_lifetime_ms = 1000.f;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::SKULL_ROCKET_FLIGHT);

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 1.f;
			trace_modifier.max_distance = 6000.f;
			trace_modifier.reference_distance = 1000.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.gain = 1.3f;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::STEEL_ROUND);

			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
			}


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
				dest_eff.particles.modifier.colorize = white;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::FURY_THROWER_TRACE);
			missile.trace_particles.modifier.colorize = rgba(202, 185, 89, 255);
			missile.trace_particles.modifier.scale_amounts = 3.f;
			missile.trace_particles.modifier.scale_lifetimes = 0.3f;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::FIRE_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.colorize = white;//{ 255, 218, 5, 255 };
			missile.damage.base = 12;
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
			trace_modifier.max_distance = 700.f;
			trace_modifier.reference_distance = 50.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.gain = 1.3f;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::ORANGE_ROUND);

			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
			}


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
				dest_eff.particles.modifier.colorize = white;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::FURY_THROWER_ATTACK);
			missile.trace_particles.modifier.colorize = rgba(255, 100, 0, 255);
			missile.trace_particles.modifier.scale_amounts = 1.1f;
			missile.trace_particles.modifier.scale_lifetimes = 0.8f;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::FIRE_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.colorize = white;//{ 255, 218, 5, 255 };
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::STEEL_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_RICOCHET);

			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::STEEL_PROJECTILE_DESTRUCTION);

			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_2));
			missile.damage.base = 49;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::STEEL_PROJECTILE_FLIGHT);

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 1.f;
			trace_modifier.max_distance = 700.f;
			trace_modifier.reference_distance = 50.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.gain = 1.3f;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::PISTOL_CYAN_ROUND);

			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
			}


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
			meta.template get<invariants::rigid_body>().damping.linear = 3.5f;

			invariants::missile missile;

			missile.ricochet_born_cooldown_ms = 17.f;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.spawn_exploding_ring = false;
				dest_eff.particles.modifier.colorize = cyan;
				dest_eff.particles.modifier.scale_amounts = 0.2f;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::PISTOL_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);
			missile.trace_particles.modifier.colorize = white;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::PISTOL_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.colorize = cyan;
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_RICOCHET);

			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_DISCHARGE_EXPLOSION);
			missile.damage.base = 12;
			missile.damage.shake *= 0.85f;
			missile.max_lifetime_ms = 450.f;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_PROJECTILE_FLIGHT);

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 0.6f;
			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.gain = 1.3f;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::SHOTGUN_RED_ROUND);

			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
			}


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
			meta.template get<invariants::rigid_body>().damping.linear = 3.0f;

			invariants::missile missile;

			missile.ricochet_born_cooldown_ms = 17.f;

			{
				auto& dest_eff = missile.damage.effects.destruction;
				dest_eff.spawn_exploding_ring = false;
				dest_eff.particles.modifier.colorize = red;
				dest_eff.particles.modifier.scale_amounts = 0.2f;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);
			missile.trace_particles.modifier.colorize = red;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::FIRE_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.colorize = red;
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_RICOCHET);

			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_DISCHARGE_EXPLOSION);
			missile.damage.base = 6;
			missile.damage.shake *= 0.25f;
			missile.max_lifetime_ms = 350.f;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_PROJECTILE_FLIGHT);

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 0.6f;
			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.gain = 1.3f;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::CYAN_ROUND);

			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
			}


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
				dest_eff.particles.modifier.colorize = cyan;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);
			missile.trace_particles.modifier.colorize = cyan;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION);
			missile.trace_particles_fly_backwards = true;
			missile.muzzle_leave_particles.modifier.colorize = cyan;
			missile.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.modifier.colorize = cyan;
			missile.ricochet_born_cooldown_ms = 17.f;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_PROJECTILE_FLIGHT);
			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_DISCHARGE_EXPLOSION);
			missile.damage.base = 12;

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 0.6f;
			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.gain = 1.3f;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::ELECTRIC_MISSILE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
			}


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
				dest_eff.particles.modifier.colorize = cyan;
				dest_eff.particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION);
			}

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);
			missile.trace_particles.modifier.colorize = cyan;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.colorize = cyan;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_PROJECTILE_FLIGHT);
			missile.damage.effects.destruction.sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_DISCHARGE_EXPLOSION);

			missile.homing_towards_hostile_strength = 1.0f;
			missile.damage.base = 64;
			missile.ricochet_born_cooldown_ms = 17.f;

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.doppler_factor = 0.6f;
			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			trace_modifier.gain = 1.3f;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_remnant_bodies::CYAN_SHELL);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::CYAN_SHELL, white);
			test_flavours::add_shell_dynamic_body(meta);

			invariants::remnant remnant;
			remnant.lifetime_secs = 20.f;
			remnant.start_shrinking_when_remaining_ms = 7000.f;
			meta.set(remnant);
		}

		{
			auto& meta = get_test_flavour(flavours, test_remnant_bodies::SHOTGUN_RED_SHELL);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SHOTGUN_RED_SHELL, white);
			test_flavours::add_shell_dynamic_body(meta);

			invariants::remnant remnant;
			remnant.lifetime_secs = 20.f;
			remnant.start_shrinking_when_remaining_ms = 7000.f;
			meta.set(remnant);
		}

		{
			auto& meta = get_test_flavour(flavours, test_remnant_bodies::SKULL_ROCKET_SHELL);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SKULL_ROCKET_SHELL, white);
			test_flavours::add_shell_dynamic_body(meta);

			invariants::remnant remnant;
			remnant.lifetime_secs = 20.f;
			remnant.start_shrinking_when_remaining_ms = 2000.f;
			meta.set(remnant);
		}

		{
			auto& meta = get_test_flavour(flavours, test_remnant_bodies::STEEL_SHELL);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::STEEL_SHELL, white);
			test_flavours::add_shell_dynamic_body(meta);

			invariants::remnant remnant;
			remnant.lifetime_secs = 20.f;
			remnant.start_shrinking_when_remaining_ms = 7000.f;
			meta.set(remnant);
		}

		{
			auto& meta = get_test_flavour(flavours, test_remnant_bodies::ORANGE_SHELL);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::ORANGE_SHELL, white);
			test_flavours::add_shell_dynamic_body(meta);

			invariants::remnant remnant;
			remnant.lifetime_secs = 20.f;
			remnant.start_shrinking_when_remaining_ms = 7000.f;
			meta.set(remnant);
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::CYAN_CHARGE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

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
				cartridge.shell_trace_particles.modifier.colorize = cyan;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::CYAN_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::CYAN_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::PISTOL_CYAN_CHARGE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

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
				cartridge.shell_trace_particles.modifier.colorize = cyan;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::CYAN_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::PISTOL_CYAN_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::STEEL_CHARGE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::STEEL_CHARGE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;

			meta.set(item);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.colorize = rgba(202, 186, 89, 255);

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::STEEL_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::STEEL_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::SKULL_ROCKET);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SKULL_ROCKET, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("1.0");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;
			item.standard_price = static_cast<money_type>(1000);

			meta.set(item);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.colorize = orange;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::SKULL_ROCKET_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::SKULL_ROCKET_FLYING);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::ORANGE_CHARGE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

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
				cartridge.shell_trace_particles.modifier.colorize = rgba(202, 186, 89, 255);

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::ORANGE_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::ORANGE_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::SHOTGUN_RED_CHARGE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

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
				cartridge.shell_trace_particles.modifier.colorize = red;
				cartridge.shell_trace_particles.modifier.scale_amounts = 1.5f;

				cartridge.shell_flavour = to_entity_flavour_id(test_remnant_bodies::SHOTGUN_RED_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::SHOTGUN_RED_ROUND);

				cartridge.num_rounds_spawned = 10;
				cartridge.rounds_spread_degrees = 20.f;
				cartridge.rounds_spread_degrees_variation = 7.f;

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::BILMER2000_MAGAZINE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BILMER2000_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.34");
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
			auto& meta = get_test_flavour(flavours, test_container_items::DATUM_GUN_MAGAZINE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::DATUM_GUN_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.40");
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
				item.standard_price = 180;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::VINDICATOR_MAGAZINE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::VINDICATOR_MAGAZINE, white);
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
			auto& meta = get_test_flavour(flavours, test_container_items::KEK9_MAGAZINE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

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

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

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

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::WARX_FQ12_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.9");
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
				item.standard_price = 140;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::PRO90_MAGAZINE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::PRO90_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.5");
			charge_deposit_def.mounting_duration_ms = 300.f;
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::STEEL_CHARGE);
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
			auto& meta = get_test_flavour(flavours, test_container_items::AO44_MAGAZINE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

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
				item.standard_price = 100;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::CALICO_MAGAZINE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::CALICO_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("1.5");
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
				item.standard_price = 120;
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::LEWSII_MAGAZINE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::LEWSII_MAGAZINE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("1.0");
			charge_deposit_def.only_allow_flavour = to_entity_flavour_id(test_shootable_charges::STEEL_CHARGE);
			charge_deposit_def.mounting_duration_ms = 300.f;

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

			meta.get<invariants::text_details>().name = "Lews II magazine";
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::CYAN_ROUND_FINISHING_TRACE);
			
			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
				test_flavours::add_sprite(meta, caches, test_scene_image_id::ROUND_TRACE, cyan);
			}

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::CYAN_ROUND).get<invariants::trace>());
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::STEEL_ROUND_FINISHING_TRACE);
			
			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
				test_flavours::add_sprite(meta, caches, test_scene_image_id::STEEL_ROUND, white);
			}

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::STEEL_ROUND).get<invariants::trace>());
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::ORANGE_ROUND_FINISHING_TRACE);
			
			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
				test_flavours::add_sprite(meta, caches, test_scene_image_id::ORANGE_ROUND, white);
			}

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::ORANGE_ROUND).get<invariants::trace>());
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::SHOTGUN_RED_ROUND_FINISHING_TRACE);
			
			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
				test_flavours::add_sprite(meta, caches, test_scene_image_id::SHOTGUN_RED_ROUND, white);
			}

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::SHOTGUN_RED_ROUND).get<invariants::trace>());
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::ELECTRIC_MISSILE_FINISHING_TRACE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
				test_flavours::add_sprite(meta, caches, test_scene_image_id::ELECTRIC_MISSILE, cyan);
			}

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::ELECTRIC_MISSILE).get<invariants::trace>());
			}
		}


		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::BILMER2000);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			meta.get<invariants::text_details>().description =
				"Standard issue sample rifle."
			;

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::BILMER2000_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4500.f, 4500.f};
			gun_def.shot_cooldown_ms = 84.f;

			gun_def.shell_angular_velocity = {2.f, 14.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 2.5f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 6;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.kickback_towards_wielder = 25.f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(250);

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.052f;
			gun_def.recoil_multiplier = 0.86f;

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::BILMER_CHAMBERING);

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BILMER2000, white);
			test_flavours::add_lying_item_dynamic_body(meta).density = 0.1f;
			set_density_mult(meta, 0.75f);
			make_default_gun_container(meta, item_holding_stance::RIFLE_LIKE, 1000.f);
			meta.get<invariants::item>().standard_price = 3100;
			set_chambering_duration_ms(meta, 450.f);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			only_allow_mag(meta, test_container_items::BILMER2000_MAGAZINE);
			meta.get<invariants::item>().specific_to = faction_type::METROPOLIS;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::PRO90);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			meta.get<invariants::text_details>().description =
				"Pretty good SMG."
			;

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::PRO90_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {3800.f, 3800.f};
			gun_def.shot_cooldown_ms = 60.f;

			gun_def.shell_angular_velocity = {2.f, 14.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 1.35f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 10;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.kickback_towards_wielder = 40.f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(500);
			gun_def.recoil_multiplier = 1.1f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.052f;

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::PRO90_CHAMBERING);

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::PRO90, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 0.6f);
			make_default_gun_container(meta, item_holding_stance::PISTOL_LIKE, 1000.f, 0.f, true);
			meta.get<invariants::item>().standard_price = 2900;
			set_chambering_duration_ms(meta, 400.f);

			auto& item = meta.get<invariants::item>();
			item.wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_SMG_DRAW);
			only_allow_mag(meta, test_container_items::PRO90_MAGAZINE);
			item.flip_when_reloading = false;
			item.draw_mag_over_when_reloading = false;
		}
		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::VINDICATOR);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			meta.get<invariants::text_details>().description =
				"Standard issue sample rifle."
			;

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::VINDICATOR_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4500.f, 4500.f};
			gun_def.shot_cooldown_ms = 100.f;

			gun_def.shell_angular_velocity = {2.f, 14.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 2.5f * 1.2f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 6;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.kickback_towards_wielder = 70.f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.062f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.recoil_multiplier = 1.05f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(400);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.shoot_animation = to_animation_id(test_scene_plain_animation_id::VINDICATOR_SHOT);
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::RIFLE_CHAMBERING);

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::VINDICATOR_SHOT_1, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 1.15f);
			make_default_gun_container(meta, item_holding_stance::RIFLE_LIKE, 1200.f, 0.f);
			meta.get<invariants::item>().standard_price = 2900;
			set_chambering_duration_ms(meta, 550.f);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			only_allow_mag(meta, test_container_items::VINDICATOR_MAGAZINE);
			meta.get<invariants::item>().specific_to = faction_type::RESISTANCE;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::LEWSII);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			meta.get<invariants::text_details>().description =
				"Standard issue sample rifle."
			;

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::VINDICATOR_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {3900.f, 3900.f};
			gun_def.shot_cooldown_ms = 60.f;

			gun_def.shell_angular_velocity = {2.f, 14.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 2.2f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 10;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.kickback_towards_wielder = 80.f;

			gun_def.heavy_heat_start_sound.id = to_sound_id(test_scene_sound_id::HEAVY_HEAT_START);
			gun_def.light_heat_start_sound.id = to_sound_id(test_scene_sound_id::LIGHT_HEAT_START);

			gun_def.steam_burst_schedule_mult = 1.f;
			gun_def.heat_cooldown_speed_mult = 4.f;

			gun_def.minimum_heat_to_shoot = 3.8f;
			gun_def.maximum_heat = 4.0f;
			gun_def.gunshot_adds_heat = 0.2f;
			gun_def.firing_engine_sound.modifier.pitch = 0.4f;
			gun_def.recoil_multiplier = 1.7f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(150);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			//gun_def.shoot_animation = to_animation_id(test_scene_plain_animation_id::VINDICATOR_SHOT);
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::RIFLE_CHAMBERING);

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::LEWSII, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			make_default_gun_container(meta, item_holding_stance::HEAVY_LIKE, 2000.f, 0.f, false, "0.01", true);
			set_density_mult(meta, 1.25);

			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::LEWSII_DRAW);
			meta.get<invariants::item>().standard_price = 4700;
			set_chambering_duration_ms(meta, 700.f);

			only_allow_mag(meta, test_container_items::LEWSII_MAGAZINE);
			meta.get<invariants::text_details>().name = "Lews II";
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::DATUM_GUN);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			meta.get<invariants::text_details>().description =
				"Standard issue sample rifle."
			;

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::PLASMA_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4400.f, 4400.f};
			gun_def.shot_cooldown_ms = 120.f;

			gun_def.shell_angular_velocity = {2.f, 14.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 2.5f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 6;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.kickback_towards_wielder = 80.f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.072f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.recoil_multiplier = 1.3f;

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.shoot_animation = to_animation_id(test_scene_plain_animation_id::DATUM_GUN_SHOT);
			gun_def.adversarial.knockout_award = static_cast<money_type>(350);
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_CHAMBERING);

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::DATUM_GUN, white);

			test_flavours::add_lying_item_dynamic_body(meta);
			set_density_mult(meta, 1.45f);

			make_default_gun_container(meta, item_holding_stance::RIFLE_LIKE, 1500.f, 0.f, true);
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::PLASMA_DRAW);
			meta.get<invariants::item>().standard_price = 4000;
			set_chambering_duration_ms(meta, 900.f);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			only_allow_mag(meta, test_container_items::DATUM_GUN_MAGAZINE);
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::KEK9);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::KEK9_MUZZLE);

			gun_def.action_mode = gun_action_type::SEMI_AUTOMATIC;
			gun_def.muzzle_velocity = {5500.f, 5500.f};
			gun_def.shot_cooldown_ms = 100.f;

			gun_def.shell_angular_velocity = {2.f, 14.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 1.55f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 6;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.042f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::MEDIUM_PISTOL_CHAMBERING);

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::KEK9, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			make_default_gun_container(meta, item_holding_stance::PISTOL_LIKE, 1000.f, 0.f, false);
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_PISTOL_DRAW);
			meta.get<invariants::item>().standard_price = 500;
			gun_def.adversarial.knockout_award = static_cast<money_type>(350);
			set_chambering_duration_ms(meta, 390.f);
			set_density_mult(meta, 0.8f);
			only_allow_mag(meta, test_container_items::KEK9_MAGAZINE);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			meta.get<invariants::item>().specific_to = faction_type::RESISTANCE;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::SN69);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::SN69_MUZZLE);

			gun_def.action_mode = gun_action_type::SEMI_AUTOMATIC;
			gun_def.muzzle_velocity = {5500.f, 5500.f};
			gun_def.shot_cooldown_ms = 90.f;

			gun_def.shell_angular_velocity = {2.f, 10.f};
			gun_def.shell_spread_degrees = 12.f;
			gun_def.shell_velocity = {300.f, 1900.f};
			gun_def.damage_multiplier = 1.2f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 6;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.recoil_multiplier = 0.9f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(350);

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.040;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::LIGHT_PISTOL_CHAMBERING);

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SN69, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			make_default_gun_container(meta, item_holding_stance::PISTOL_LIKE, 1100.f, 0.f, true);
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_PISTOL_DRAW);
			meta.get<invariants::item>().standard_price = 500;
			set_chambering_duration_ms(meta, 350.f);
			set_density_mult(meta, 0.7f);
			only_allow_mag(meta, test_container_items::SN69_MAGAZINE);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			meta.get<invariants::item>().specific_to = faction_type::METROPOLIS;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::AO44);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::AO44_MUZZLE);

			gun_def.action_mode = gun_action_type::BOLT_ACTION;
			gun_def.muzzle_velocity = {4500.f, 4500.f};
			gun_def.shot_cooldown_ms = 0.f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::REVOLVER_CHAMBERING);
			gun_def.allow_chambering_with_akimbo = true;

			gun_def.shell_angular_velocity = {2.f, 10.f};
			gun_def.shell_spread_degrees = 12.f;
			gun_def.shell_velocity = {500.f, 2500.f};
			gun_def.damage_multiplier = 1.2f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 2;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.recoil_multiplier = 2.f;
			gun_def.kickback_towards_wielder = 70.f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.202f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.adversarial.knockout_award = static_cast<money_type>(700);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::AO44, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			make_default_gun_container(meta, item_holding_stance::PISTOL_LIKE, 1200.f, 0.f, false, "0.1");
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_PISTOL_DRAW);
			meta.get<invariants::item>().standard_price = 700;
			set_chambering_duration_ms(meta, 300.f);
			set_density_mult(meta, 0.9f);
			meta.template get<invariants::item>().space_occupied_per_charge = to_space_units("3.5");
			only_allow_mag(meta, test_container_items::AO44_MAGAZINE);
			meta.get<invariants::item>().draw_mag_over_when_reloading = true;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::ELON_HRL);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::ELON_HRL_MUZZLE);

			gun_def.action_mode = gun_action_type::BOLT_ACTION;
			gun_def.muzzle_velocity = {5200.f, 5200.f};
			gun_def.shot_cooldown_ms = 0.f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::AUTOMATIC_SHOTGUN_CHAMBERING);
			gun_def.allow_charge_in_chamber_magazine_when_chamber_loaded = false;

			gun_def.shell_angular_velocity = {2.f, 10.f};
			gun_def.shell_spread_degrees = 12.f;
			gun_def.shell_velocity = {500.f, 2500.f};
			gun_def.damage_multiplier = 1.f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 0;
			gun_def.recoil_multiplier = 4.f;
			gun_def.kickback_towards_wielder = 500.f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.202f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.adversarial.knockout_award = static_cast<money_type>(100);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.allow_chambering_with_akimbo = true;

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::ELON_HRL, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			make_default_gun_container(meta, item_holding_stance::HEAVY_LIKE, 0.f, 0.f, false, "1.0");

			auto& slots = meta.get<invariants::container>().slots;

			only_allow_chamber_charge(meta, test_shootable_charges::SKULL_ROCKET);

			slots[slot_function::GUN_CHAMBER].physical_behaviour = slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY;
			slots.erase(slot_function::GUN_DETACHABLE_MAGAZINE);
			slots[slot_function::GUN_CHAMBER].never_reachable_for_mounting = true;

			{
				auto& mag = slots[slot_function::GUN_CHAMBER_MAGAZINE];
				mag = slots[slot_function::GUN_CHAMBER];
				mag.never_reachable_for_mounting = false;
				mag.mounting_duration_ms = 2400.f;
				mag.start_mounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_RIFLE_START_LOAD);
				mag.finish_mounting_sound.id = to_sound_id(test_scene_sound_id::STANDARD_RIFLE_FINISH_LOAD);
			}

			set_chambering_duration_ms(meta, 700.f);

			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_PISTOL_DRAW);
			meta.get<invariants::item>().standard_price = 6500;
			set_density_mult(meta, 1.9f);
			meta.template get<invariants::item>().space_occupied_per_charge = to_space_units("12.0");

			meta.get<invariants::item>().draw_mag_over_when_reloading = true;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::WARX_FQ12);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::WARX_FQ12_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = { 4500.f, 6600.f };
			gun_def.shot_cooldown_ms = 250.f;
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::AUTOMATIC_SHOTGUN_CHAMBERING);
			gun_def.allow_chambering_with_akimbo = false;

			gun_def.shell_angular_velocity = {2.f, 10.f};
			gun_def.shell_spread_degrees = 12.f;
			gun_def.shell_velocity = {900.f, 3500.f};
			gun_def.damage_multiplier = 1.f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 3;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.recoil_multiplier = 2.3f;
			gun_def.kickback_towards_wielder = 130.f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.242f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.adversarial.knockout_award = static_cast<money_type>(150);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::WARX_FQ12, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			make_default_gun_container(meta, item_holding_stance::RIFLE_LIKE, 1400.f, 0.f, false, "0.1");
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_GUN_DRAW);
			meta.get<invariants::item>().standard_price = 2900;
			set_chambering_duration_ms(meta, 650.f);
			set_density_mult(meta, 1.3f);
			meta.template get<invariants::item>().space_occupied_per_charge = to_space_units("7.5");
			only_allow_mag(meta, test_container_items::WARX_FQ12_MAGAZINE);
			meta.get<invariants::item>().draw_mag_over_when_reloading = false;
			meta.get<invariants::item>().flip_when_reloading = true;
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::CALICO);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::CALICO_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4100.f, 4100.f};
			gun_def.shot_cooldown_ms = 110.f;

			gun_def.shell_angular_velocity = {2.f, 10.f};
			gun_def.shell_spread_degrees = 12.f;
			gun_def.shell_velocity = {300.f, 1900.f};
			gun_def.damage_multiplier = 0.82f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 6;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.recoil_multiplier = 1.25f;
			gun_def.kickback_towards_wielder = 30.f;
			gun_def.adversarial.knockout_award = static_cast<money_type>(550);

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.072f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.shoot_animation = to_animation_id(test_scene_plain_animation_id::CALICO_SHOT);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.chambering_sound.id = to_sound_id(test_scene_sound_id::CALICO_CHAMBERING);

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::CALICO, white);
			test_flavours::add_lying_item_dynamic_body(meta);
			make_default_gun_container(meta, item_holding_stance::PISTOL_LIKE, 1500.f, 0.f, false, "0.1");
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_PISTOL_DRAW);
			meta.get<invariants::item>().standard_price = 650;
			set_chambering_duration_ms(meta, 500.f);
			set_density_mult(meta, 0.7f);
			only_allow_mag(meta, test_container_items::CALICO_MAGAZINE);
			meta.get<invariants::item>().draw_mag_over_when_reloading = true;
			meta.get<invariants::item>().specific_to = faction_type::SPECTATOR;
		}
		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::AMPLIFIER_ARM);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::ASSAULT_RIFLE_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {2000.f, 2000.f};
			gun_def.shot_cooldown_ms = 300.f;

			gun_def.damage_multiplier = 0.5f;

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.magic_missile_flavour = to_entity_flavour_id(test_plain_missiles::ELECTRIC_MISSILE);
			gun_def.adversarial.knockout_award = static_cast<money_type>(150);

			meta.set(gun_def);

			default_gun_props(meta);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::AMPLIFIER_ARM, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("3.0");
			item.holding_stance = item_holding_stance::RIFLE_LIKE;
			item.gratis_ammo_pieces_with_first = 0;
			meta.set(item);
			meta.get<invariants::item>().standard_price = 2500;
			set_density_mult(meta, 0.8f);
		}
	}
}