#include "test_scenes/ingredients/ingredients.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

#include "game/assets/ids/asset_ids.h"
#include "test_scenes/test_scene_particle_effects.h"
#include "test_scenes/test_scene_animations.h"

#include "game/components/sender_component.h"
#include "game/components/missile_component.h"
#include "game/components/item_component.h"
#include "game/components/melee_component.h"

inventory_space_type to_space_units(const std::string& s);

namespace test_flavours {
	void populate_melee_flavours(const populate_flavours_input in) {
		auto& flavours = in.flavours;
		auto& caches = in.caches;

		auto make_knife = [&](
			const auto flavour_id,
			const auto image_id,
			const auto price,
			const auto specific_to,
			const auto weight_mult,
			const auto color,
			const float dmg_mult = 1.f,
			const money_type award = 1350,
			const float swings_mult = 1.0f
		) -> auto& {
			auto& meta = get_test_flavour(flavours, flavour_id);

			test_flavours::add_sprite(meta, caches, image_id, white);

			{
				auto& fixtures = test_flavours::add_lying_item_dynamic_body(meta);
				fixtures.density *= 1.5f * weight_mult;
				fixtures.restitution *= 1.5f;
				fixtures.material = to_physical_material_id(test_scene_physical_material_id::KNIFE);
			}

			{
				invariants::item item;

				item.space_occupied_per_charge = to_space_units("1.5");
				item.holding_stance = item_holding_stance::KNIFE_LIKE;
				item.wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_DRAW);
				item.standard_price = static_cast<money_type>(price);
				item.specific_to = specific_to;
				item.categories_for_slot_compatibility.set(item_category::SHOULDER_WEARABLE);
				item.wear_sound.id = to_sound_id(test_scene_sound_id::SHEATH_KNIFE);
				item.draw_over_hands = false;

				meta.set(item);
			}

			{
				invariants::melee melee;
				melee.adversarial.knockout_award = static_cast<money_type>(award);

				{
					auto& t = melee.throw_def;
					t.min_speed_to_hurt = 500.f;
					// t.throw_angular_speed = 360.f * 10.f;
					t.throw_angular_speed = 0.f;

					auto& d = t.damage;
					d.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
					d.base = int(88.f * weight_mult * dmg_mult);
					auto& eff = d.effects;

					eff.sentience_impact.sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_SENTIENCE_IMPACT);
					eff.sentience_impact.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_IMPACT);
					t.boomerang_impulse.linear = 4000.f;

					{
						auto& clash = t.clash;

						clash.sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_CLASH);
						clash.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_CLASH);
					}
				}

				{
					auto& a = melee.actions[weapon_action_type::PRIMARY];
					a.init_sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_PRIMARY);
					a.init_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_ATTACK);
					a.init_particles.modifier.color = color;
					a.wielder_init_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_PRIMARY_SMOKE);
					a.obstacle_hit_rotation_inertia_ms = 1000.f;
					a.obstacle_hit_recoil_mult = 1.1f * weight_mult;
					a.obstacle_hit_kickback_impulse = 2500.f;
					a.obstacle_hit_linear_inertia_ms = 140.f;
					a.wielder_impulse = 387.f * weight_mult;
					a.wielder_inert_for_ms = 200.f * weight_mult;
					a.cooldown_ms = 200.f * weight_mult * swings_mult;
					a.cp_required = static_cast<int>(3.f * weight_mult);

					a.obstacle_hit_recoil = 40.f;
					a.sentience_hit_recoil = 10.f * weight_mult;

					a.returning_animation_on_finish = false;

					{
						auto& clash = a.clash;

						clash.sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_CLASH);
						clash.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_CLASH);
						clash.victim_inert_for_ms = 500.f * weight_mult;
						clash.impulse = 450.f * weight_mult;
					}

					a.damage.base = int(27.f * weight_mult * dmg_mult);
					a.damage.shake.strength *= 0.9f;
					a.damage.impact_impulse = 20.f;
					a.damage.impulse_multiplier_against_sentience = 10.f;
					a.bonus_damage_speed_ratio = 1.f / 1700.f;
					a.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
					a.head_radius_multiplier = 0.9f;

					auto& eff = a.damage.effects;

					eff.impact.sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_PRIMARY_IMPACT);
					eff.impact.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_IMPACT);

					eff.sentience_impact.sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_SENTIENCE_IMPACT);
				}

				{
					auto& a = melee.actions[weapon_action_type::SECONDARY];
					a.init_sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_SECONDARY);
					a.init_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_ATTACK);
					a.init_particles.modifier.color = color;
					a.damage.shake.strength *= 1.15f;
					a.wielder_init_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_SECONDARY_SMOKE);
					a.wielder_impulse = 717.f;
					a.wielder_inert_for_ms = 300.f * weight_mult;
					a.cooldown_ms = 500.f * weight_mult * swings_mult;
					a.obstacle_hit_linear_inertia_ms = 100.f;
					a.cp_required = static_cast<int>(6.f * weight_mult);
					a.obstacle_hit_recoil_mult = 1.2f * weight_mult;

					a.obstacle_hit_kickback_impulse = 3600.f;
					a.obstacle_hit_rotation_inertia_ms = 1000.f;
					a.obstacle_hit_recoil = 80.f * weight_mult;
					a.sentience_hit_recoil = 20.f * weight_mult;

					a.returning_animation_on_finish = true;

					{
						auto& clash = a.clash;

						clash.sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_CLASH);
						clash.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_CLASH);
						clash.victim_inert_for_ms = 850.f * weight_mult;
						clash.impulse = 750.f * weight_mult;
					}

					a.damage.base = int(50.f * weight_mult * dmg_mult);
					a.damage.impact_impulse = 40.f;
					a.damage.impulse_multiplier_against_sentience = 10.f;
					a.bonus_damage_speed_ratio = 1.f / 1700.f;
					a.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
					a.head_radius_multiplier = 0.9f;

					auto& eff = a.damage.effects;

					eff.impact.sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_SECONDARY_IMPACT);
					eff.impact.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_IMPACT);

					eff.sentience_impact.sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_SENTIENCE_IMPACT);
				}

				meta.set(melee);
			}

			return meta;
		};

		{
			auto& meta = make_knife(
				test_melee_weapons::FURY_THROWER,
				test_scene_image_id::FURY_THROWER,
				static_cast<money_type>(1000),
				faction_type::RESISTANCE,
				1.f,
				white,
				1.125f,
				4500,
				1.15f
			);

			invariants::continuous_particles particles_def;
			particles_def.effect_id = to_particle_effect_id(test_scene_particle_effect_id::FURY_THROWER_TRACE);
			meta.set(particles_def);

			components::continuous_particles particles;
			particles.modifier.color = orange;
			meta.set(particles);

			for (auto& a : meta.get<invariants::melee>().actions) {
				a.init_particles.id = to_particle_effect_id(test_scene_particle_effect_id::FURY_THROWER_ATTACK);
			}
		}

		make_knife(
			test_melee_weapons::ELECTRIC_RAPIER,
			test_scene_image_id::ELECTRIC_RAPIER,
			static_cast<money_type>(1100),
			faction_type::SPECTATOR,
			0.65f,
			white,
			1.8f,
			static_cast<money_type>(800)
		);

		make_knife(
			test_melee_weapons::CYAN_SCYTHE,
			test_scene_image_id::CYAN_SCYTHE,
			static_cast<money_type>(700),
			faction_type::METROPOLIS,
			1.f,
			cyan,
			1.f,
			static_cast<money_type>(2000)
		);

		make_knife(
			test_melee_weapons::YELLOW_DAGGER,
			test_scene_image_id::YELLOW_DAGGER,
			static_cast<money_type>(700),
			faction_type::RESISTANCE,
			1.f,
			yellow,
			1.f,
			static_cast<money_type>(2000)
		);

		{
			auto& meta = make_knife(
				test_melee_weapons::MINI_KNIFE,
				test_scene_image_id::MINI_KNIFE,
				static_cast<money_type>(500),
				faction_type::SPECTATOR,
				0.7f,
				white,
				1.0f,
				static_cast<money_type>(5000)
			);

			for (auto& a : meta.get<invariants::melee>().actions) {
				a.init_particles.id = {};
			}

			meta.template get<invariants::melee>().throw_def.damage.base = 49.f;
		}

		{
			auto& meta = make_knife(
				test_melee_weapons::POSEIDON,
				test_scene_image_id::POSEIDON,
				static_cast<money_type>(1000),
				faction_type::SPECTATOR,
				1.1f,
				white,
				1.0f,
				4500
			);
			
			invariants::continuous_particles particles_def;
			particles_def.effect_id = to_particle_effect_id(test_scene_particle_effect_id::POSEIDON_THROWER_TRACE);
			meta.set(particles_def);

			components::continuous_particles particles;
			particles.modifier.color = rgba(0, 200, 255, 255);
			meta.set(particles);

			for (auto& a : meta.get<invariants::melee>().actions) {
				a.init_particles.id = to_particle_effect_id(test_scene_particle_effect_id::POSEIDON_THROWER_ATTACK);
			}
		}

		{
			auto& meta = make_knife(
				test_melee_weapons::ASSAULT_RATTLE,
				test_scene_image_id::ASSAULT_RATTLE_1,
				static_cast<money_type>(1700),
				faction_type::SPECTATOR,
				1.8f,
				cyan
			);

			auto& melee = meta.template get<invariants::melee>();

			melee.throw_def.damage.effects.sentience_impact.sound.id = to_sound_id(test_scene_sound_id::ASSAULT_RATTLE_SECONDARY_IMPACT);
			melee.throw_def.clash.sound.id = to_sound_id(test_scene_sound_id::ASSAULT_RATTLE_CLASH);
			melee.throw_def.throw_angular_speed = 360.f * 25.f;
			melee.throw_def.head_radius_multiplier = 0.7f;

			for (auto& a : melee.actions) {
				a.head_radius_multiplier = 0.7f;
			}

			melee.adversarial.knockout_award = static_cast<money_type>(700); 

			{
				auto& a = melee.actions[weapon_action_type::PRIMARY];
				a.init_sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_PRIMARY);
				a.clash.sound.id = to_sound_id(test_scene_sound_id::ASSAULT_RATTLE_CLASH);

				auto& eff = a.damage.effects;
				eff.impact.sound.id = to_sound_id(test_scene_sound_id::ASSAULT_RATTLE_SECONDARY_IMPACT);
				eff.sentience_impact.sound.id = to_sound_id(test_scene_sound_id::ASSAULT_RATTLE_SECONDARY_IMPACT);
			}

			{
				auto& a = melee.actions[weapon_action_type::SECONDARY];
				a.init_sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_SECONDARY);
				a.clash.sound.id = to_sound_id(test_scene_sound_id::ASSAULT_RATTLE_CLASH);

				auto& eff = a.damage.effects;
				eff.impact.sound.id = to_sound_id(test_scene_sound_id::ASSAULT_RATTLE_SECONDARY_IMPACT);
				eff.sentience_impact.sound.id = to_sound_id(test_scene_sound_id::ASSAULT_RATTLE_SECONDARY_IMPACT);
			}

			auto& snd = meta.template get<invariants::continuous_sound>().effect;
			snd.id = to_sound_id(test_scene_sound_id::ASSAULT_RATTLE_HUMMING);
			snd.modifier.gain *= 0.15f;

			auto& modifier = snd.modifier;

			modifier.distance_model = augs::distance_model::LINEAR_DISTANCE_CLAMPED;
			modifier.reference_distance = 50.f;
			modifier.max_distance = 150.f;

			invariants::animation anim;
			anim.id = to_animation_id(test_scene_plain_animation_id::ASSAULT_RATTLE);
			meta.set(anim);

			meta.template get<invariants::fixtures>().material = to_physical_material_id(test_scene_physical_material_id::METAL);
			meta.template get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::ASSAULT_RATTLE_DRAW);
			meta.template get<invariants::item>().space_occupied_per_charge = to_space_units("3.0");
		}
	}
}
