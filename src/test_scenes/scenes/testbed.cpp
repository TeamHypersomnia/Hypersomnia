/*
	Disable float/int warnings, this is just a content script
*/
#if PLATFORM_WINDOWS
#pragma warning(disable : 4244)
#endif
#include "augs/templates/algorithm_templates.h"
#include "augs/math/cascade_aligner.h"
#include "game/assets/ids/asset_ids.h"
#include "game/assets/all_logical_assets.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"

#include "test_scenes/test_scene_flavours.h"
#include "test_scenes/scenes/testbed.h"
#include "test_scenes/ingredients/ingredients.h"
#include "test_scenes/test_scenes_content.h"

#include "game/cosmos/cosmos.h"
#include "game/organization/all_component_includes.h"
#include "game/organization/all_messages_includes.h"

#include "game/stateless_systems/input_system.h"
#include "game/stateless_systems/particles_existence_system.h"
#include "game/stateless_systems/car_system.h"
#include "game/stateless_systems/driver_system.h"

#include "game/enums/faction_type.h"
#include "game/detail/describers.h"

#include "game/messages/intent_message.h"
#include "game/detail/inventory/perform_transfer.h"

#include "view/viewables/image_cache.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/cosmic_delta.h"

#include "test_scenes/scenes/test_scene_node.h"
#include "game/modes/test_scene_mode.h"
#include "game/modes/bomb_mode.h"

namespace test_scenes {
	void testbed::setup(test_scene_mode_vars& vars) {
		vars.name = "Testbed vars";
		vars.spawned_faction = faction_type::RESISTANCE;

		const auto vindicator_mag = requested_ammo { 
			to_entity_flavour_id(test_container_items::SAMPLE_MAGAZINE), 
			to_entity_flavour_id(test_shootable_charges::STEEL_CHARGE)
		};

		vars.initial_eq.weapon = to_entity_flavour_id(test_shootable_weapons::VINDICATOR);
		vars.initial_eq.weapon_ammo = vindicator_mag;
		vars.initial_eq.personal_deposit_wearable = to_entity_flavour_id(test_container_items::STANDARD_PERSONAL_DEPOSIT);

		vars.initial_eq.spare_mags.emplace_back(3, vindicator_mag);
	}

	void testbed::setup(bomb_mode_vars& vars) {
		vars.name = "Testbed bomb vars";

		auto& resistance = vars.factions[faction_type::RESISTANCE];

		const auto vindicator_mag = requested_ammo { 
			to_entity_flavour_id(test_container_items::SAMPLE_MAGAZINE), 
			to_entity_flavour_id(test_shootable_charges::STEEL_CHARGE)
		};

		const auto bilmer_mag = requested_ammo { 
			to_entity_flavour_id(test_container_items::SAMPLE_MAGAZINE), 
			to_entity_flavour_id(test_shootable_charges::CYAN_CHARGE)
		};

		resistance.initial_eq.weapon = to_entity_flavour_id(test_shootable_weapons::VINDICATOR);
		resistance.initial_eq.weapon_ammo = vindicator_mag;
		resistance.initial_eq.personal_deposit_wearable = to_entity_flavour_id(test_container_items::STANDARD_PERSONAL_DEPOSIT);
		resistance.initial_eq.spare_mags.emplace_back(3, vindicator_mag);

		auto& metropolis = vars.factions[faction_type::METROPOLIS];

		metropolis.initial_eq.weapon = to_entity_flavour_id(test_shootable_weapons::SAMPLE_RIFLE);
		metropolis.initial_eq.weapon_ammo = bilmer_mag;
		metropolis.initial_eq.personal_deposit_wearable = to_entity_flavour_id(test_container_items::STANDARD_PERSONAL_DEPOSIT);
		metropolis.initial_eq.belt_wearable = to_entity_flavour_id(test_tool_items::DEFUSE_KIT);
		metropolis.initial_eq.spare_mags.emplace_back(3, bilmer_mag);

		{
			auto& mt = vars.view.event_sounds[faction_type::METROPOLIS];

			mt[battle_event::START] = to_sound_id(test_scene_sound_id::MT_START);
			mt[battle_event::BOMB_PLANTED] = to_sound_id(test_scene_sound_id::MT_BOMB_PLANTED);
			mt[battle_event::BOMB_DEFUSED] = to_sound_id(test_scene_sound_id::MT_BOMB_DEFUSED);
			mt[battle_event::ITS_TOO_LATE_RUN] = to_sound_id(test_scene_sound_id::MT_ITS_TOO_LATE_RUN);
		}

		{
			auto& re = vars.view.event_sounds[faction_type::RESISTANCE];
			re = vars.view.event_sounds[faction_type::METROPOLIS];
		}

		{
			auto& mt = vars.view.win_sounds[faction_type::METROPOLIS];

			mt[faction_type::RESISTANCE] = to_sound_id(test_scene_sound_id::MT_RESISTANCE_WINS);
			mt[faction_type::METROPOLIS] = to_sound_id(test_scene_sound_id::MT_METROPOLIS_WINS);
		}

		{
			auto& re = vars.view.win_sounds[faction_type::RESISTANCE];
			re = vars.view.win_sounds[faction_type::METROPOLIS];
		}

		vars.bomb_flavour = to_entity_flavour_id(test_hand_explosives::BOMB);

		vars.view.logos[faction_type::METROPOLIS] = to_image_id(test_scene_image_id::METROPOLIS_LOGO);
		vars.view.logos[faction_type::ATLANTIS] = to_image_id(test_scene_image_id::ATLANTIS_LOGO);
		vars.view.logos[faction_type::RESISTANCE] = to_image_id(test_scene_image_id::RESISTANCE_LOGO);

		vars.view.square_logos[faction_type::METROPOLIS] = to_image_id(test_scene_image_id::METROPOLIS_SQUARE_LOGO);
		// TODO: add atlantis logo
		vars.view.square_logos[faction_type::ATLANTIS] = to_image_id(test_scene_image_id::METROPOLIS_SQUARE_LOGO);
		vars.view.square_logos[faction_type::RESISTANCE] = to_image_id(test_scene_image_id::RESISTANCE_SQUARE_LOGO);

		{
			vars.view.icons[scoreboard_icon_type::DEATH_ICON] = to_image_id(test_scene_image_id::DEATH_ICON);
			vars.view.icons[scoreboard_icon_type::UNCONSCIOUS_ICON] = to_image_id(test_scene_image_id::UNCONSCIOUS_ICON);
			vars.view.icons[scoreboard_icon_type::NO_AMMO_ICON] = to_image_id(test_scene_image_id::NO_AMMO_ICON);
			vars.view.icons[scoreboard_icon_type::BOMB_ICON] = to_image_id(test_scene_image_id::BOMB_ICON);
			vars.view.icons[scoreboard_icon_type::DEFUSE_KIT_ICON] = to_image_id(test_scene_image_id::DEFUSE_KIT_ICON);
		}
	}

	void testbed::populate(const loaded_image_caches_map& caches, const logic_step step) const {
		auto& world = step.get_cosmos();
		
		auto create = [&](auto&&... args) {
			return create_test_scene_entity(world, std::forward<decltype(args)>(args)...);
		};

		const auto crate_type = test_plain_sprited_bodys::CRATE;
		const auto force_type = test_hand_explosives::FORCE_GRENADE;
		const auto ped_type = test_hand_explosives::PED_GRENADE;
		const auto interference_type = test_hand_explosives::INTERFERENCE_GRENADE;

		const auto sample_backpack = test_container_items::SAMPLE_BACKPACK;
		const auto brown_backpack = test_container_items::BROWN_BACKPACK;

#if TODO
		const auto car = prefabs::create_car(step, transformr( { 1490, 340 }, -180));
		const auto car2 = prefabs::create_car(step, transformr({ 1490, 340 + 400 }, -180));
		const auto car3 = prefabs::create_car(step, transformr({ 1490, 340 + 800 }, -180));

		const auto riding_car = prefabs::create_car(step, transformr({ 850, 1000 }, -90));

		const auto riding_car2 = prefabs::create_car(step, transformr({ -850 + 1000, -8200 }, -90 + 180));
#endif

		for (int i = 0; i < 10; ++i) {
			create(force_type, vec2{ 254, 611 + i *100.f });
			create(ped_type, vec2{ 204, 611 + i * 100.f });
			create(interference_type, vec2{ 154, 611 + i * 100.f });
		}

		{
			std::vector<transformr> spawn_transforms = {
				{ { 318, 267 }, -90 },
				{ { 638, 267 }, -90 }
			};

			for (const auto& s : spawn_transforms) {
				create(test_point_markers::RESISTANCE_SPAWN, s);
			}
		}

		{
			std::vector<transformr> spawn_transforms = {
				{ { 0, -930 }, 90 },
				{ { 1041, -833 }, 90 }
			};

			for (const auto& s : spawn_transforms) {
				create(test_point_markers::METROPOLIS_SPAWN, s);
			}
		}

		std::vector<transformr> character_transforms = {
			{ { 0, 300 }, 0 },
			{ { -1540, 211 }, 68 },
			{ { 1102, 213 }, 110 },
			{ { 1102, 413 }, 110 },
			{ { -100, 20000 }, 0 },
			{ { 1200, 15000 }, 0 },
			{ { -300, 20000 }, 0 },
			{ { -300, -2000 }, 0 },
			{ { -400, -2000 }, 0 },
			{ { -500, -2000 }, 0 }
		};

		const auto num_characters = character_transforms.size();

		auto get_size_of = [&caches](const auto id) {
			return vec2i(caches.at(to_image_id(id)).get_original_size());
		};

		std::vector<entity_id> new_characters;
		new_characters.resize(num_characters);

		auto character = [&](const size_t i) {
			return i < new_characters.size() ? world[new_characters.at(i)] : world[entity_id()];
		};

		const auto metropolis_type = test_controlled_characters::METROPOLIS_SOLDIER;
		const auto resistance_type = test_controlled_characters::RESISTANCE_SOLDIER;

		for (std::size_t i = 0; i < num_characters; ++i) {
			auto transform = character_transforms[i];

			const bool is_metropolis = i % 2 == 0;
			const auto new_character = create(is_metropolis ? metropolis_type : resistance_type, transform);

			new_characters[i] = new_character;

			auto& sentience = new_character.get<components::sentience>();

			sentience.get<consciousness_meter_instance>().set_maximum_value(400);
			sentience.get<consciousness_meter_instance>().set_value(400);

			sentience.get<personal_electricity_meter_instance>().set_maximum_value(400);
			sentience.get<personal_electricity_meter_instance>().set_value(400);

			new_character.get<components::attitude>().maximum_divergence_angle_before_shooting = 25;

			if (i == 0 || i == 1) {
				/* Let's have two OP characters */
				sentience.get<health_meter_instance>().set_value(10000);
				sentience.get<health_meter_instance>().set_maximum_value(10000);

				sentience.get<personal_electricity_meter_instance>().set_value(10000);
				sentience.get<personal_electricity_meter_instance>().set_maximum_value(10000);

				const auto rifle = (is_metropolis ? prefabs::create_sample_rifle : prefabs::create_vindicator)(
					step, vec2(100, -500), prefabs::create_sample_magazine(step, vec2(100, -650), prefabs::create_steel_charge(step, vec2(0, 0)), 30)
				);

				if (i == 1) {
					new_character.get<components::crosshair>().base_offset.x = -500;
				}

				perform_transfer(item_slot_transfer_request::standard(rifle, new_character.get_primary_hand()), step);
			}

			if (i == 7 || i == 8 || i == 9) {
				/* Give some stuff to three test characters */
				if (i == 9) {
					const auto rifle = prefabs::create_sample_rifle(step, vec2(100, -500),
						prefabs::create_sample_magazine(step, vec2(100, -650),
							prefabs::create_cyan_charge(step, vec2(0, 0))));

					perform_transfer(item_slot_transfer_request::standard(rifle, new_character.get_primary_hand()), step);
				}
				else {
					const auto rifle = (i == 7 ? prefabs::create_sample_rifle : prefabs::create_sample_rifle)(step, vec2(100, -500),
						prefabs::create_sample_magazine(step, vec2(100, -650),
							prefabs::create_cyan_charge(step, vec2(0, 0))));

					perform_transfer(item_slot_transfer_request::standard(rifle, new_character.get_primary_hand()), step);
				}

				auto backpack = create(is_metropolis ? sample_backpack : brown_backpack, vec2(200, -650));
				perform_transfer(item_slot_transfer_request::standard(backpack, new_character[slot_function::BACK]), step);
			}

			fill_range(sentience.learned_spells, true);
		}

		{
			const vec2 coords[] = {
				{ 1200, 5400 },
				{ 1200, 10400 },
				{ 1200, 15400 },
				{ 1200, 20400 },
				{ -1, 25400 },
				{ 1200, 30400 },
			};

			for (const auto c : coords) {
				create(crate_type, c + vec2(-100, 400) );
				create(crate_type, c + vec2(300, 300) );
				create(crate_type, c + vec2(100, -200) );

				const auto light_pos = c + vec2(0, 100);
				const auto light_cyan = c.x < 0 ? orange : rgba(30, 255, 255, 255);

				{
					const auto l = create(test_static_lights::STRONG_LAMP, transformr(light_pos));
					auto& light = l.get<components::light>();
					light.color = light_cyan;
				}

				{
					create(test_wandering_pixels_decorations::WANDERING_PIXELS, [&](const auto e, auto&&...){
						auto& w = e.template get<components::wandering_pixels>();

						w.colorize = light_cyan;
						w.particles_count = 150;

						const auto reach = xywh(light_pos.x- 350, light_pos.y-350, 500, 500);
						e.set_logical_size(reach.get_size());
						e.set_logic_transform(reach.get_center());
					});
				}
			}
		}

		{
			{
				const auto l = create(test_static_lights::STRONG_LAMP, transformr(vec2(-44, 270)));
				auto& light = l.get<components::light>();
				light.color = cyan;
			}
			{
				const auto l = create(test_static_lights::STRONG_LAMP, transformr(vec2(1098, 220)));
				auto& light = l.get<components::light>();
				light.color = orange;
			}
			{
				const auto l = create(test_static_lights::STRONG_LAMP, transformr(vec2(223, -47)));
				auto& light = l.get<components::light>();
				light.color = cyan;
			}

			{
				const auto left_reach = xywh(164.f - 8.f + 90.f - 550, 220 - 250, 1000, 600);
				const auto right_reach = xywh(1164.f - 8.f + 90.f - 600, 220 - 250, 1000, 600);

				{
					create(test_wandering_pixels_decorations::WANDERING_PIXELS, [&](const auto e, auto&&...){

						auto& w = e.template get<components::wandering_pixels>();

						w.colorize = cyan;
						w.particles_count = 50;
						e.set_logical_size(left_reach.get_size());
						e.set_logic_transform(left_reach.get_center());
					});
				}

				{
					create(test_wandering_pixels_decorations::WANDERING_PIXELS, [&](const auto e, auto&&...){

						auto& w = e.template get<components::wandering_pixels>();

						w.colorize = orange;
						w.particles_count = 50;
						e.set_logical_size(right_reach.get_size());
						e.set_logic_transform(right_reach.get_center());
					});
				}
			}


			{
				create(test_sprite_decorations::HAVE_A_PLEASANT, transformr(vec2(-42, -32)));
				create(test_sprite_decorations::AWAKENING, transformr(vec2(-42, 8)));
				create(test_sprite_decorations::METROPOLIS, transformr(vec2(1106, 3)));

				//const vec2 floor_size = get_size_of(test_scene_image_id::FLOOR);
				const auto total_floor_size = vec2i(1280, 1280);
				const auto floor_origin = vec2(512, -768);

				auto floor_align = [&](const auto flavour_id) {
					return make_cascade_aligner(
						floor_origin,
						total_floor_size, 
						test_scene_node { world, flavour_id }
					);
				};

				floor_align(test_sprite_decorations::WATER_ROOM_FLOOR).set_size(total_floor_size);
				floor_align(test_plain_sprited_bodys::BRICK_WALL)
					.ro().ti().stretch_b().again()
					.ro().bo().extend_l(2).extend_b(1).again()
					.lo().ti().stretch_b().again()
					.lo().bo().extend_r(2).extend_b(1).next(test_sprite_decorations::ROAD_DIRT)
					.ro().bi().next(test_sprite_decorations::ROAD)
					.mult_size({ 1, 38 }).bo()
				;

				{
					const auto soil_origin = vec2(-750, 37000);
					const auto total_soil_size = vec2i(15000, 85000);

					create(test_sprite_decorations::SOIL, transformr(soil_origin)).set_logical_size(total_soil_size);
				}
			}
		}

		for (int k = 0; k < 2; ++k) {
			prefabs::create_kek9(step, vec2(-800 - k * 150, -200),
				prefabs::create_sample_magazine(step, vec2(100, -650),
					prefabs::create_cyan_charge(step, vec2(0, 0))));

			prefabs::create_sn69(step, vec2(-800 - k * 150, 0),
				prefabs::create_sample_magazine(step, vec2(100, -650),
					prefabs::create_cyan_charge(step, vec2(0, 0))));

			prefabs::create_ao44(step, vec2(-800 - k * 150, 200),
				prefabs::create_ao44_magazine(step, vec2(100, -650),
					prefabs::create_ao44_charge(step, vec2(0, 0))));

			prefabs::create_gun(step, vec2(-800 - k * 150, 400), 
			test_shootable_weapons::PRO90, prefabs::create_pro90_magazine(step, vec2(100, -650), prefabs::create_steel_charge(step, vec2(0, 0)), 50));
		}

		prefabs::create_amplifier_arm(step, vec2(-300, -500 + 50));
		
		/* TODO: Spawn a machete actually */
		const auto machete_type = force_type;

		create(machete_type, vec2(100, 100));

		create(sample_backpack, vec2(200, -750));
		create(brown_backpack, vec2(280, -750));
		create(test_complex_decorations::ROTATING_FAN, vec2(380, -750));

		prefabs::create_gun(step, vec2(280, -750), test_shootable_weapons::DATUM_GUN, prefabs::create_sample_magazine(step, vec2(100, -650), prefabs::create_cyan_charge(step, vec2(0, 0)), 25));

		prefabs::create_vindicator(
			step, vec2(280, -150), prefabs::create_sample_magazine(step, vec2(100, -650), prefabs::create_steel_charge(step, vec2(0, 0)), 30)
		);

		prefabs::create_sample_rifle(
			step, vec2(280, -250), prefabs::create_sample_magazine(step, vec2(100, -650), prefabs::create_cyan_charge(step, vec2(0, 0)), 30)
		);

		prefabs::create_gun(step, vec2(300, -100), test_shootable_weapons::LEWSII, prefabs::create_magazine(step, vec2(100, -650), test_container_items::LEWSII_MAGAZINE, prefabs::create_steel_charge(step, vec2(0, 0)), 100));
		prefabs::create_gun(step, vec2(400, -100), test_shootable_weapons::LEWSII, prefabs::create_magazine(step, vec2(100, -650), test_container_items::LEWSII_MAGAZINE, prefabs::create_steel_charge(step, vec2(0, 0)), 100));

		const auto aquarium_size = get_size_of(test_scene_image_id::AQUARIUM_SAND_1);
		const auto whole_aquarium_size = aquarium_size * 2;

		auto create_aquarium = [&](const transformr aquarium_tr) {
			const auto aquarium_origin = aquarium_tr + transformr(aquarium_size / 2);

			auto aquarium_align = [&](const auto flavour_id) {
				return make_cascade_aligner(
					aquarium_origin.pos, 
					whole_aquarium_size, 
					test_scene_node { world, flavour_id }
				);
			};

			{
				{
					const auto bub = test_particles_decorations::FLOWER_BUBBLES;
					const auto flpink = test_complex_decorations::FLOWER_PINK;

					aquarium_align(flpink)
						.li().ti().nr().nr().nd().prepend(bub).dup()
						.nd().nd().prepend(bub).dup()
						.nl().nd().prepend(bub)
						.again(test_complex_decorations::FLOWER_CYAN)
						.ri().ti().nl().nd().nd().nd().prepend(bub).dup()
						.nl().nd().prepend(bub).dup()
						.nd().prepend(bub)
					;

					aquarium_align(test_complex_decorations::PINK_CORAL)
						.ti().nl().nd().nd().nd().dup()
						.nd().nl().nl().rot_90().next(flpink).ro().prepend(bub).dup().nu().prepend(bub)
					;
				}

				aquarium_align(test_sound_decorations::AQUARIUM_AMBIENCE_LEFT)
					.lo().bo()
					.again(test_sound_decorations::AQUARIUM_AMBIENCE_RIGHT)
					.ro().bo()
					.again(test_sound_decorations::LOUDY_FAN)
					.to()
					.again(test_sprite_decorations::LAB_WALL_A2).flip_v()
					.to()
				;

				aquarium_align(test_particles_decorations::AQUARIUM_BUBBLES).ti().rot_90().mv(0, 15);

				aquarium_align(test_plain_sprited_bodys::AQUARIUM_GLASS)
					.li().bo().nr()
					.next(test_plain_sprited_bodys::AQUARIUM_GLASS_START).lo().create_pop()
					.stretch_r(-1)
					.next(test_plain_sprited_bodys::AQUARIUM_GLASS_START).ro().flip_h()
				;

				aquarium_align(test_plain_sprited_bodys::LAB_WALL_SMOOTH_END)
					.li().bo().again()
					.ri().bo().flip_h()
				;

				aquarium_align(test_plain_sprited_bodys::LAB_WALL_CORNER_SQUARE)
					.lo().bo().again()
					.ro().bo().flip_h().again()

					.to().lo().flip_v().again()
					.to().ro().flip_v().flip_h()
				;

				aquarium_align(test_plain_sprited_bodys::LAB_WALL)
					.rot_90().lo().bi().stretch_t().again()
					.flip_v().rot_90().ro().bi().stretch_t().again()
					.flip_v().to().li().stretch_r()
				;
			}

			{
				vec2 lights[3] = {
					{ -145, 417 },
					vec2(193, 161) - vec2(160, 206),
					{ 463, -145 }
				};

				rgba light_cols[3] = { 
					rgba(0, 132, 190, 255),
					rgba(0, 99, 126, 255),
					rgba(0, 180, 59, 255)
				};

				for (int i = 0; i < 3; ++i) {
					const auto l = create(test_static_lights::AQUARIUM_LAMP);
					l.set_logic_transform(aquarium_tr + transformr(lights[i]));
					auto& light = l.get<components::light>();
					light.color = light_cols[i];
				}
			}

			{
				constexpr int N = 9;
				constexpr int DN = 4;

				transformr caustics[N] = {
					{ { 16, 496 }, 0 },
					{ { 16, 369 }, -75 },
					{ { -26, 250 }, 45 },

					{ { 2, 98 }, 75 },
					{ { 96, 74 }, 0 },
					{ { 151, -88 }, 120 },

					{ { 296, -104 }, 150 },
					{ { 357, -26 }, 45 },
					{ { 513, -1 }, 195 }
				};

				int caustics_offsets [N] =  {
					28, 2, 11,
					28, 2, 11,
					28, 2, 11
				};

				transformr dim_caustics[DN] = {
					{ { 225, 446 }, -75 },
					{ { 241, 238 }, -15 },
					{ { 449, 414 }, -75 },
					{ { 465, 238 }, -135 }
				};

				int dim_caustics_offsets [DN] = {
					31, 20, 10, 13
				};

				for (int i = 0; i < N; ++i) {
					const auto target = caustics[i] + aquarium_tr;
					auto ent = create(test_complex_decorations::WATER_SURFACE, target);
					ent.get<components::animation>().state.frame_num = caustics_offsets[i];
				}

				for (int i = 0; i < DN; ++i) {
					const auto target = dim_caustics[i] + aquarium_tr;
					auto ent = create(test_complex_decorations::WATER_SURFACE, target);
					ent.get<components::animation>().state.frame_num = dim_caustics_offsets[i];
					ent.get<components::sprite>().colorize.a = 79;
				}
			}

			{
				constexpr int N = 2;

				transformr halogens[N] = {
					{ { -174, 417 }, 90 },
					{ { 463, -174 }, 180 }
				};

				rgba halogens_light_cols[N] = {
					rgba(96, 255, 255, 255),
					rgba(103, 255, 69, 255)
				};

				rgba halogens_bodies_cols[N] = {
					rgba(0, 122, 255, 255),
					rgba(0, 180, 59, 255)
				};

				for (int i = 0; i < N; ++i) {
					const auto target = halogens[i] + aquarium_tr;
					
					{
						auto ent = create(test_sprite_decorations::AQUARIUM_HALOGEN_1_BODY, target);
						ent.get<components::sprite>().colorize = halogens_bodies_cols[i];
					}

					{
						auto ent = create(test_sprite_decorations::AQUARIUM_HALOGEN_1_LIGHT, target);
						ent.get<components::sprite>().colorize = halogens_light_cols[i];
					}
				}
			}

			{
				const auto bottom_lamp_tr = transformr(vec2(193, 161) - vec2(160, 206), -45.f);

				const auto target = bottom_lamp_tr + aquarium_tr;

				{
					auto ent = create(test_sprite_decorations::AQUARIUM_BOTTOM_LAMP_BODY, target);
					ent.get<components::sprite>().colorize = rgba(0, 122, 255, 255);
				}

				{
					auto ent = create(test_sprite_decorations::AQUARIUM_BOTTOM_LAMP_LIGHT, target);
					ent.get<components::sprite>().colorize = rgba(96, 255, 255, 255);
				}
			}


			create(test_wandering_pixels_decorations::WANDERING_PIXELS, [&](const auto e, auto&&...){
				auto& w = e.template get<components::wandering_pixels>();

				w.colorize = cyan;
				w.particles_count = 40;
				w.keep_particles_within_bounds = true;
				e.set_logical_size(vec2(750, 750));
				e.set_logic_transform(aquarium_origin);
			});

			create(test_wandering_pixels_decorations::AQUARIUM_PIXELS_LIGHT, [&](const auto e, auto&&...){
				e.set_logic_transform(aquarium_origin);
			});

			create(test_wandering_pixels_decorations::AQUARIUM_PIXELS_DIM, [&](const auto e, auto&&...){
				e.set_logic_transform(aquarium_origin);
			});

			{
				const auto edge_size = get_size_of(test_scene_image_id::AQUARIUM_SAND_EDGE);

				const auto s = edge_size;
				const auto h = edge_size / 2;

				for (int g = 0; g < (aquarium_size * 2).x / s.x; ++g) {
					create(test_sprite_decorations::AQUARIUM_SAND_EDGE, aquarium_origin.pos + aquarium_size + vec2(-h.x, -h.y) - vec2(s.x * g, 0));
				}
			}

			create(test_sprite_decorations::WATER_COLOR_OVERLAY, aquarium_origin);

			create(test_sprite_decorations::AQUARIUM_SAND_1, aquarium_tr);
			create(test_sprite_decorations::AQUARIUM_SAND_1, aquarium_tr + transformr(vec2(aquarium_size.x, 0)));
			create(test_sprite_decorations::AQUARIUM_SAND_2, aquarium_tr + transformr(vec2(aquarium_size.x, aquarium_size.y)));
			create(test_sprite_decorations::AQUARIUM_SAND_2, aquarium_tr + transformr(vec2(0, aquarium_size.y)));

			create(test_sprite_decorations::DUNE_SMALL, transformr(aquarium_origin.pos + vec2(-193, -193) + vec2(52, -22)));
			create(test_sprite_decorations::DUNE_SMALL, transformr(aquarium_origin.pos + vec2(-237, 255)));
			create(test_sprite_decorations::DUNE_BIG, transformr(aquarium_origin.pos + vec2(-74, -48)));
			create(test_sprite_decorations::DUNE_BIG, transformr(aquarium_origin.pos + vec2(161, 126)));

			const auto yellowfish = test_complex_decorations::YELLOW_FISH;
			const auto darkbluefish = test_complex_decorations::DARKBLUE_FISH;
			const auto cyanvioletfish = test_complex_decorations::CYANVIOLET_FISH;
			const auto jellyfish = test_complex_decorations::JELLYFISH;
			const auto dragon_fish = test_complex_decorations::DRAGON_FISH;
			const auto rainbow_dragon_fish = test_complex_decorations::RAINBOW_DRAGON_FISH;

			auto create_fish = [&](auto t, auto where, auto origin) {
				const auto decor = create(t, where);
				decor.template get<components::movement_path>().origin = origin;
				const auto secs = real32(decor.template get<components::animation>().state.frame_num) * 12.23f;
				decor.template get<components::sprite>().effect_offset_secs = secs;
				return decor;
			};

			create_fish(yellowfish, aquarium_tr - vec2(80, 10), aquarium_origin);
			create_fish(yellowfish, aquarium_tr + transformr(vec2(80, 10), -180), aquarium_origin);
			create_fish(yellowfish, aquarium_tr - vec2(80, 30), aquarium_origin);
			create_fish(yellowfish, aquarium_tr + transformr(vec2(80, 50), -180), aquarium_origin);
			create_fish(yellowfish, aquarium_tr - vec2(120, 30), aquarium_origin);
			create_fish(yellowfish, aquarium_tr + transformr(vec2(90, 40), -180), aquarium_origin);

			create_fish(cyanvioletfish, aquarium_tr - vec2(40, 10), aquarium_origin);
			create_fish(cyanvioletfish, aquarium_tr + transformr(vec2(40, 10), -180), aquarium_origin);
			create_fish(cyanvioletfish, aquarium_tr - vec2(40, 30), aquarium_origin);
			create_fish(cyanvioletfish, aquarium_tr + transformr(vec2(40, 50), -180), aquarium_origin);
			create_fish(cyanvioletfish, aquarium_tr - vec2(70, 30), aquarium_origin);
			create_fish(cyanvioletfish, aquarium_tr + transformr(vec2(40, 40), -180), aquarium_origin);

			create_fish(yellowfish, aquarium_tr + vec2(20, 20) - vec2(80, 10), aquarium_origin);
			create_fish(yellowfish, aquarium_tr + vec2(20, 20) + transformr(vec2(80, 10), -180), aquarium_origin);
			create_fish(yellowfish, aquarium_tr + vec2(20, 20) - vec2(80, 30), aquarium_origin);
			create_fish(yellowfish, aquarium_tr + vec2(20, 20) + transformr(vec2(80, 50), -180), aquarium_origin);
			create_fish(yellowfish, aquarium_tr + vec2(20, 20) - vec2(120, 30), aquarium_origin);
			create_fish(yellowfish, aquarium_tr + vec2(20, 20) + transformr(vec2(90, 40), -180), aquarium_origin);

			const auto jellyfishtr = aquarium_tr + transformr(vec2(100, 100), -45);
			create_fish(darkbluefish, jellyfishtr - vec2(80, 10), aquarium_origin);
			create_fish(darkbluefish, jellyfishtr + transformr(vec2(80, 10), -180), aquarium_origin);

			create_fish(darkbluefish, jellyfishtr - vec2(80, 30), aquarium_origin);
			create_fish(darkbluefish, jellyfishtr + transformr(vec2(80, 50), -180), aquarium_origin);

			create_fish(darkbluefish, jellyfishtr - vec2(120, 30), aquarium_origin);
			create_fish(darkbluefish, jellyfishtr + transformr(vec2(90, 40), -180), aquarium_origin);

			create_fish(jellyfish, aquarium_tr - vec2(180, 30), aquarium_origin);
			create_fish(jellyfish, aquarium_tr + transformr(vec2(190, 40), -180), aquarium_origin);
			create_fish(jellyfish, aquarium_tr - vec2(180, 50), aquarium_origin);
			create_fish(jellyfish, aquarium_tr + transformr(vec2(190, 60), -180), aquarium_origin);
			create_fish(jellyfish, aquarium_tr - vec2(180, 70), aquarium_origin);
			create_fish(jellyfish, aquarium_tr + transformr(vec2(190, 80), -180), aquarium_origin);

			create_fish(dragon_fish, aquarium_tr - vec2(280, 130), aquarium_origin);
			create_fish(dragon_fish, aquarium_tr + transformr(vec2(290, 40), -180), aquarium_origin);
			create_fish(dragon_fish, aquarium_tr - vec2(280, 150), aquarium_origin);
			create_fish(dragon_fish, aquarium_tr + transformr(vec2(290, 60), -180), aquarium_origin);

			create_fish(rainbow_dragon_fish, vec2(40, 40) + aquarium_tr.pos - vec2(280, 130), aquarium_origin);
			create_fish(rainbow_dragon_fish, vec2(40, 40) + aquarium_tr.pos + vec2(290, 40), aquarium_origin);
			create_fish(rainbow_dragon_fish, vec2(40, 40) + aquarium_tr.pos - vec2(280, 150), aquarium_origin);
			create_fish(rainbow_dragon_fish, vec2(40, 40) + aquarium_tr.pos + vec2(290, 60), aquarium_origin);
		};

		const auto orig1 = vec2(380, -1524);
		create_aquarium(orig1);

		create(test_box_markers::BOMBSITE_A, vec2(580, -800)).set_logical_size(vec2(600, 200));
		create(test_box_markers::BOMBSITE_B, vec2(480, 200)).set_logical_size(vec2(600, 200));

		//create(test_hand_explosives::BOMB, vec2(280, 200));

		const auto lab_wall_size = get_size_of(test_scene_image_id::LAB_WALL);

		make_cascade_aligner(
			orig1 + aquarium_size / 2, 
			whole_aquarium_size + vec2i::square(2 * lab_wall_size.y),
			test_scene_node { world, test_complex_decorations::CONSOLE_LIGHT }
		).ro()
		.next(test_sound_decorations::HUMMING_DISABLED);

		if (character(2).alive()) {
			const auto second_machete = create(machete_type, vec2(0, 300));
			perform_transfer(item_slot_transfer_request::standard(second_machete, character(2).get_primary_hand()), step);
		}
	}
}
