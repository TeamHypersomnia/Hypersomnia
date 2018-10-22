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

		{
			auto& meta = get_test_flavour(flavours, test_melee_weapons::RESISTANCE_KNIFE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::RESISTANCE_KNIFE, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			{
				invariants::item item;

				item.space_occupied_per_charge = to_space_units("1.5");
				item.holding_stance = item_holding_stance::KNIFE_LIKE;
				item.wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_DRAW);

				meta.set(item);
			}

			{
				invariants::melee melee;

				{
					auto& t = melee.throw_def;
					t.min_speed_to_hurt = 500.f;
				}

				{
					auto& a = melee.actions[weapon_action_type::PRIMARY];
					a.init_sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_PRIMARY);
					a.wielder_impulse = 20.f;
					a.cp_required = 5.f;

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
					a.damage.impulse = 1000.f;
					a.damage.impact_sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_SECONDARY);
					a.damage.impact_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_IMPACT);
					a.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
				}

				{
					auto& a = melee.actions[weapon_action_type::SECONDARY];
					a.init_sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_SECONDARY);
					a.wielder_impulse = 50.f;
					a.cp_required = 12.f;

					a.obstacle_hit_recoil = 80.f;
					a.sentience_hit_recoil = 20.f;

					a.returning_animation_on_finish = true;

					{
						auto& clash = a.clash;

						clash.sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_CLASH);
						clash.particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_CLASH);
					}

					a.damage.base = 57.f;
					a.damage.impulse = 2000.f;
					a.damage.impact_sound.id = to_sound_id(test_scene_sound_id::STANDARD_KNIFE_SECONDARY);
					a.damage.impact_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STANDARD_KNIFE_IMPACT);
					a.damage.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
				}

				meta.set(melee);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_melee_weapons::METROPOLIS_KNIFE);

			meta = get_test_flavour(flavours, test_melee_weapons::RESISTANCE_KNIFE);
		}
	}
}
