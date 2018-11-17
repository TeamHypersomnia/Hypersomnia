#include "test_scenes/ingredients/ingredients.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

#include "game/assets/ids/asset_ids.h"
#include "test_scenes/test_scene_particle_effects.h"

#include "game/components/sender_component.h"
#include "game/components/missile_component.h"
#include "game/components/item_component.h"
#include "game/components/melee_component.h"

#include "game/detail/inventory/perform_transfer.h"

namespace test_flavours {
	void populate_melee_flavours(const populate_flavours_input in) {
		auto& flavours = in.flavours;
		auto& caches = in.caches;

		auto make_knife = [&](
			const auto f,
			const auto i,
			const auto price,
			const auto specific_to,
			const auto weight_ratio,
			const auto color
		) -> auto& {
			(void)weight_ratio;

			auto& meta = get_test_flavour(flavours, f);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, i, white);

			{
				auto& fixtures = test_flavours::add_lying_item_dynamic_body(meta);
				fixtures.density *= 1.5f;
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

				meta.set(item);
			}

			{
				invariants::melee melee;

				{
					auto& t = melee.throw_def;
					t.min_speed_to_hurt = 500.f;

					auto& d = t.damage;
					d.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
					d.base = 88.f;
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
					a.init_particles.modifier.colorize = color;
					a.wielder_init_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_PRIMARY_SMOKE);
					a.obstacle_hit_rotation_inertia_ms = 1000.f;
					a.obstacle_hit_recoil_mult = 1.1f;
					a.obstacle_hit_kickback_impulse = 2500.f;
					a.obstacle_hit_linear_inertia_ms = 140.f;
					a.wielder_impulse = 1000.f;
					a.wielder_inert_for_ms = 200.f;
					a.cooldown_ms = 250.f;
					a.cp_required = 1.f;

					a.obstacle_hit_recoil = 40.f;
					a.sentience_hit_recoil = 10.f;

					a.returning_animation_on_finish = false;

					{
						auto& clash = a.clash;

						clash.sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_CLASH);
						clash.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_CLASH);
					}

					a.damage.base = 27.f;
					a.damage.shake *= 0.4f;
					a.damage.impact_impulse = 20.f;
					a.damage.impulse_multiplier_against_sentience = 10.f;
					a.bonus_damage_speed_ratio = 1.f / 1700.f;
					a.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

					auto& eff = a.damage.effects;

					eff.impact.sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_PRIMARY_IMPACT);
					eff.impact.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_IMPACT);

					eff.sentience_impact.sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_SENTIENCE_IMPACT);
				}

				{
					auto& a = melee.actions[weapon_action_type::SECONDARY];
					a.init_sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_SECONDARY);
					a.init_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_ATTACK);
					a.init_particles.modifier.colorize = color;
					a.wielder_init_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_SECONDARY_SMOKE);
					a.wielder_impulse = 950.f;
					a.wielder_inert_for_ms = 300.f;
					a.obstacle_hit_linear_inertia_ms = 100.f;
					a.cp_required = 2.f;

					a.obstacle_hit_kickback_impulse = 3600.f;
					a.obstacle_hit_rotation_inertia_ms = 1000.f;
					a.obstacle_hit_recoil = 80.f;
					a.sentience_hit_recoil = 20.f;

					a.returning_animation_on_finish = true;

					{
						auto& clash = a.clash;

						clash.sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_CLASH);
						clash.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_CLASH);
					}

					a.damage.base = 57.f;
					a.damage.impact_impulse = 40.f;
					a.damage.impulse_multiplier_against_sentience = 10.f;
					a.bonus_damage_speed_ratio = 1.f / 1700.f;
					a.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

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
				static_cast<money_type>(250),
				faction_type::RESISTANCE,
				1.1f,
				white
			);

			invariants::continuous_particles particles_def;
			particles_def.effect.id = to_particle_effect_id(test_scene_particle_effect_id::FURY_THROWER_TRACE);
			particles_def.effect.modifier.colorize = orange;
			meta.set(particles_def);

			for (auto& a : meta.get<invariants::melee>().actions) {
				a.init_particles.id = to_particle_effect_id(test_scene_particle_effect_id::FURY_THROWER_ATTACK);
			}
		}

		make_knife(
			test_melee_weapons::ELECTRIC_RAPIER,
			test_scene_image_id::ELECTRIC_RAPIER,
			static_cast<money_type>(350),
			faction_type::SPECTATOR,
			1.2f,
			white
		);

		make_knife(
			test_melee_weapons::CYAN_SCYTHE,
			test_scene_image_id::CYAN_SCYTHE,
			static_cast<money_type>(300),
			faction_type::METROPOLIS,
			1.f,
			cyan
		);

		{
			auto& meta = make_knife(
				test_melee_weapons::POSEIDON,
				test_scene_image_id::POSEIDON,
				static_cast<money_type>(350),
				faction_type::SPECTATOR,
				1.2f,
				white
			);
			
			invariants::continuous_particles particles_def;
			particles_def.effect.id = to_particle_effect_id(test_scene_particle_effect_id::POSEIDON_THROWER_TRACE);
			particles_def.effect.modifier.colorize = rgba(0, 200, 255, 255);
			meta.set(particles_def);

			for (auto& a : meta.get<invariants::melee>().actions) {
				a.init_particles.id = to_particle_effect_id(test_scene_particle_effect_id::POSEIDON_THROWER_ATTACK);
			}
		}
	}
}
