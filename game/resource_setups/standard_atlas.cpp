#include "all.h"
#include "game/resources/manager.h"
#include "augs/graphics/shader.h"

namespace resource_setups {
	std::unordered_map<assets::texture_id, source_image_information> load_standard_images() {
		std::unordered_map<assets::texture_id, source_image_information> output;

		{
			auto& in = output[assets::texture_id::TEST_CROSSHAIR];
			in.textures[image_map_type::DIFFUSE] = { "hypersomnia/gfx/crosshair.png", assets::atlas_id::GAME_WORLD_ATLAS };
		}

		{
			auto& in = output[assets::texture_id::TEST_PLAYER];
			in.textures[image_map_type::DIFFUSE] = { "hypersomnia/gfx/walk_1.png", assets::atlas_id::GAME_WORLD_ATLAS };
		}

		{
			auto& in = output[assets::texture_id::TEST_BACKGROUND];
			in.textures[image_map_type::DIFFUSE] = { "hypersomnia/gfx/snow_textures/snow3.png", assets::atlas_id::GAME_WORLD_ATLAS };
		}

		{
			auto& in = output[assets::texture_id::CRATE];
			in.textures[image_map_type::DIFFUSE] = { "hypersomnia/gfx/crate2.png", assets::atlas_id::GAME_WORLD_ATLAS };
		}

		return std::move(output);
	}

	std::unordered_map<assets::font_id, source_font_location> load_standard_fonts() {
		return {
			{
				assets::font_id::GUI_FONT,

				{
					{
						"hypersomnia/Kubasta.ttf",
						L" ABCDEFGHIJKLMNOPRSTUVWXYZQabcdefghijklmnoprstuvwxyzq0123456789.!@#$%^&*()_+-=[];'\\,./{}:\"|<>?~",
						16
					},

					assets::atlas_id::GAME_WORLD_ATLAS
				}
			}
		};
	}

	requested_atlas_resources request_standard_atlas_resources() {
		requested_atlas_resources output;

		output.request(assets::texture_id::TEST_CROSSHAIR, "hypersomnia/gfx/crosshair.png");
		output.request(assets::texture_id::TEST_PLAYER, "hypersomnia/gfx/walk_1.png");
		output.request(assets::texture_id::TEST_BACKGROUND, "hypersomnia/gfx/snow_textures/snow3.png");
		output.request(assets::texture_id::CRATE, "hypersomnia/gfx/crate2.png");
		output.request(assets::texture_id::CRATE_DESTROYED, "hypersomnia/gfx/crate2_destroyed.png");
		output.request(assets::texture_id::CAR_INSIDE, "hypersomnia/gfx/crate2.png");
		output.request(assets::texture_id::CAR_FRONT, "hypersomnia/gfx/crate2.png");
		output.request(assets::texture_id::JMIX114, "hypersomnia/gfx/jmix114.png");
		output.request(assets::texture_id::TRUCK_FRONT, "hypersomnia/gfx/truck_front.png");
		output.request(assets::texture_id::TRUCK_INSIDE, "hypersomnia/gfx/truck_inside.png");
		output.request(assets::texture_id::MENU_GAME_LOGO, "hypersomnia/gfx/menu_game_logo.png");
		output.request(assets::texture_id::ASSAULT_RIFLE, "hypersomnia/gfx/assault_rifle.png");
		output.request(assets::texture_id::BILMER2000, "hypersomnia/gfx/bilmer2000.png");
		output.request(assets::texture_id::KEK9, "hypersomnia/gfx/kek9.png");
		output.request(assets::texture_id::SHOTGUN, "hypersomnia/gfx/shotgun.png");
		output.request(assets::texture_id::SUBMACHINE, "hypersomnia/gfx/submachine.png");
		output.request(assets::texture_id::PISTOL, "hypersomnia/gfx/pistol.png");
		output.request(assets::texture_id::URBAN_CYAN_MACHETE, "hypersomnia/gfx/urban_cyan_machete.png");
		output.request(assets::texture_id::AMPLIFIER_ARM, "hypersomnia/gfx/amplifier_arm.png");
		output.request(assets::texture_id::SAMPLE_MAGAZINE, "hypersomnia/gfx/magazine.png");
		output.request(assets::texture_id::SMALL_MAGAZINE, "hypersomnia/gfx/small_magazine.png");
		output.request(assets::texture_id::SAMPLE_SUPPRESSOR, "hypersomnia/gfx/suppressor.png");
		output.request(assets::texture_id::ROUND_TRACE, "hypersomnia/gfx/round_trace.png");
		output.request(assets::texture_id::ENERGY_BALL, "hypersomnia/gfx/energy_ball.png");
		output.request(assets::texture_id::PINK_CHARGE, "hypersomnia/gfx/pink_charge.png");
		output.request(assets::texture_id::PINK_SHELL, "hypersomnia/gfx/pink_shell.png");
		output.request(assets::texture_id::CYAN_CHARGE, "hypersomnia/gfx/cyan_charge.png");
		output.request(assets::texture_id::CYAN_SHELL, "hypersomnia/gfx/cyan_shell.png");
		output.request(assets::texture_id::GREEN_CHARGE, "hypersomnia/gfx/green_charge.png");
		output.request(assets::texture_id::GREEN_SHELL, "hypersomnia/gfx/green_shell.png");
		output.request(assets::texture_id::BACKPACK, "hypersomnia/gfx/backpack.png");

		output.request(assets::texture_id::ATTACHMENT_CIRCLE_FILLED, "hypersomnia/gfx/backpack.png");
		output.request(assets::texture_id::ATTACHMENT_CIRCLE_BORDER, "hypersomnia/gfx/backpack.png");
		output.request(assets::texture_id::ACTION_BUTTON_FILLED, "hypersomnia/gfx/backpack.png");
		output.request(assets::texture_id::ACTION_BUTTON_BORDER, "hypersomnia/gfx/backpack.png");
		output.request(assets::texture_id::HUD_CIRCULAR_BAR_MEDIUM, "hypersomnia/gfx/backpack.png");

		//{
		//	augs::image attachment_circle_filled;
		//	attachment_circle_filled.paint_filled_circle(16);
		//
		//	augs::image attachment_circle_border;
		//	attachment_circle_border.paint_circle_midpoint(16);
		//
		//	output.request(assets::texture_id::ATTACHMENT_CIRCLE_FILLED, attachment_circle_filled);
		//	output.request(assets::texture_id::ATTACHMENT_CIRCLE_BORDER, attachment_circle_border);
		//}
		//
		//{
		//	augs::image action_circle_filled;
		//	action_circle_filled.paint_filled_circle(19);
		//
		//	augs::image action_circle_border;
		//	action_circle_border.paint_circle_midpoint(19);
		//
		//	output.request(assets::texture_id::ACTION_BUTTON_FILLED, action_circle_filled);
		//	output.request(assets::texture_id::ACTION_BUTTON_BORDER, action_circle_border);
		//}

		//augs::image hud_circular_hud_medium;
		//hud_circular_hud_medium.paint_circle_midpoint(57, 1, cyan, false, false, vec2().set_from_degrees(-45), vec2().set_from_degrees(45));
		//hud_circular_hud_medium.paint_circle_midpoint(55, 5, white, false, false, vec2().set_from_degrees(135), vec2().set_from_degrees(-180 + 30));
		//
		//output.request(assets::texture_id::HUD_CIRCULAR_BAR_MEDIUM, hud_circular_hud_medium);

		output.request_button_with_corners(
			assets::texture_id::HOTBAR_BUTTON_INSIDE,
			"generated/gui/hotbar_button"
		);

		output.request_button_with_corners(
			assets::texture_id::MENU_BUTTON_INSIDE,
			"generated/gui/menu_button"
		);

		output.request(assets::texture_id::CONTAINER_OPEN_ICON, "hypersomnia/gfx/container_open_icon.png");
		output.request(assets::texture_id::CONTAINER_CLOSED_ICON, "hypersomnia/gfx/container_closed_icon.png");

		output.request(assets::texture_id::GUI_CURSOR, "hypersomnia/gfx/gui_cursor.png");
		output.request(assets::texture_id::GUI_CURSOR_HOVER, "hypersomnia/gfx/gui_cursor_hover.png");
		output.request(assets::texture_id::GUI_CURSOR_ADD, "hypersomnia/gfx/gui_cursor_add.png");
		output.request(assets::texture_id::GUI_CURSOR_ERROR, "hypersomnia/gfx/gui_cursor_error.png");
		output.request(assets::texture_id::GUI_CURSOR_MINUS, "hypersomnia/gfx/gui_cursor_minus.png");

		output.request(assets::texture_id::BLANK, "hypersomnia/gfx/blank.png");
		output.request(assets::texture_id::LASER, "hypersomnia/gfx/laser.png");
		output.request(assets::texture_id::LASER_GLOW_EDGE, "hypersomnia/gfx/laser_glow_edge.png");
		output.request(assets::texture_id::DROP_HAND_ICON, "hypersomnia/gfx/drop_hand_icon.png");
		output.request(assets::texture_id::SECONDARY_HAND_ICON, "hypersomnia/gfx/secondary_hand_icon.png");
		output.request(assets::texture_id::PRIMARY_HAND_ICON, "hypersomnia/gfx/primary_hand_icon.png");
		output.request(assets::texture_id::SHOULDER_SLOT_ICON, "hypersomnia/gfx/shoulder_slot_icon.png");
		output.request(assets::texture_id::ARMOR_SLOT_ICON, "hypersomnia/gfx/armor_slot_icon.png");
		output.request(assets::texture_id::CHAMBER_SLOT_ICON, "hypersomnia/gfx/chamber_slot_icon.png");
		output.request(assets::texture_id::DETACHABLE_MAGAZINE_ICON, "hypersomnia/gfx/detachable_magazine_slot_icon.png");
		output.request(assets::texture_id::GUN_MUZZLE_SLOT_ICON, "hypersomnia/gfx/gun_muzzle_slot_icon.png");
		
		output.request(assets::texture_id::DEAD_TORSO, "hypersomnia/gfx/dead_torso.png");


		output.request_indexed(
			assets::texture_id::TORSO_MOVING_FIRST,
			assets::texture_id::TORSO_MOVING_LAST,
			"hypersomnia/gfx/torso_white_walk_barehands"
		);

		output.request_indexed(
			assets::texture_id::VIOLET_TORSO_MOVING_FIRST,
			assets::texture_id::VIOLET_TORSO_MOVING_LAST,
			"hypersomnia/gfx/sprite"
		);

		output.request_indexed(
			assets::texture_id::BLUE_TORSO_MOVING_FIRST,
			assets::texture_id::BLUE_TORSO_MOVING_LAST,
			"hypersomnia/gfx/walk"
		);

		output.request_indexed(
			assets::texture_id::METROPOLIS_TILE_FIRST,
			assets::texture_id::METROPOLIS_TILE_LAST,
			"hypersomnia/gfx/tileset/tile"
		);

		output.request_indexed(
			assets::texture_id::SMOKE_PARTICLE_FIRST,
			assets::texture_id(int(assets::texture_id::SMOKE_PARTICLE_FIRST) + 3),
			"hypersomnia/gfx/smoke"
		);

		output.request_indexed(
			assets::texture_id::PIXEL_THUNDER_FIRST,
			assets::texture_id::PIXEL_THUNDER_LAST,
			"hypersomnia/gfx/pixel_thunder"
		);

		output.request_indexed(
			assets::texture_id::BLINK_FIRST,
			assets::texture_id::BLINK_LAST,
			"hypersomnia/gfx/blink"
		);

		output.request_indexed(
			assets::texture_id::CAST_BLINK_FIRST,
			assets::texture_id::CAST_BLINK_LAST,
			"hypersomnia/gfx/cast_blink"
		);

		output.request(assets::texture_id::HAVE_A_PLEASANT, "hypersomnia/gfx/have_a_pleasant.png");
		output.request(assets::texture_id::AWAKENING, "hypersomnia/gfx/awakening.png");
		output.request(assets::texture_id::METROPOLIS, "hypersomnia/gfx/metropolis.png");
		output.request(assets::texture_id::BRICK_WALL, "hypersomnia/gfx/brick_wall.png");
		output.request(assets::texture_id::ROAD, "hypersomnia/gfx/road.png");
		output.request(assets::texture_id::ROAD_FRONT_DIRT, "hypersomnia/gfx/road_front_dirt.png");
		output.request(assets::texture_id::WANDERING_CROSS, "hypersomnia/gfx/wandering_cross.png");
		output.request(assets::texture_id::TRUCK_ENGINE, "hypersomnia/gfx/truck_engine.png");
		output.request(assets::texture_id::HEALTH_ICON, "hypersomnia/gfx/health_icon.png");
		output.request(assets::texture_id::PERSONAL_ELECTRICITY_ICON, "hypersomnia/gfx/personal_electricity_icon.png");
		output.request(assets::texture_id::CONSCIOUSNESS_ICON, "hypersomnia/gfx/consciousness_icon.png");
		output.request(assets::texture_id::PERK_HASTE_ICON, "hypersomnia/gfx/perk_haste_icon.png");
		output.request(assets::texture_id::SPELL_HASTE_ICON, "hypersomnia/gfx/spell_haste_icon.png");
		output.request(assets::texture_id::SPELL_FURY_OF_THE_AEONS_ICON, "hypersomnia/gfx/spell_fury_of_the_aeons_icon.png");
		output.request(assets::texture_id::SPELL_ULTIMATE_WRATH_OF_THE_AEONS_ICON, "hypersomnia/gfx/spell_ultimate_wrath_of_the_aeons_icon.png");
		output.request(assets::texture_id::SPELL_ELECTRIC_TRIAD_ICON, "hypersomnia/gfx/spell_electric_triad_icon.png");
		output.request(assets::texture_id::SPELL_BORDER, "hypersomnia/gfx/spell_border.png");
		output.request(assets::texture_id::PERK_ELECTRIC_SHIELD_ICON, "hypersomnia/gfx/perk_electric_shield_icon.png");
		output.request(assets::texture_id::SPELL_ELECTRIC_SHIELD_ICON, "hypersomnia/gfx/spell_electric_shield_icon.png");
		output.request(assets::texture_id::CAST_HIGHLIGHT, "hypersomnia/gfx/cast_highlight.png");
		output.request(assets::texture_id::GRENADE_SPOON, "hypersomnia/gfx/grenade_spoon.png");
		output.request(assets::texture_id::FORCE_GRENADE, "hypersomnia/gfx/force_grenade.png");
		output.request(assets::texture_id::PED_GRENADE, "hypersomnia/gfx/ped_grenade.png");
		output.request(assets::texture_id::INTERFERENCE_GRENADE, "hypersomnia/gfx/interference_grenade.png");
		output.request(assets::texture_id::FORCE_GRENADE_RELEASED, "hypersomnia/gfx/force_grenade_released.png");
		output.request(assets::texture_id::PED_GRENADE_RELEASED, "hypersomnia/gfx/ped_grenade_released.png");
		output.request(assets::texture_id::INTERFERENCE_GRENADE_RELEASED, "hypersomnia/gfx/interference_grenade_released.png");

		return std::move(output);
	}

	void load_standard_image_settings() {
		auto& manager = get_resource_manager();

		{
			image_usage_settings s;
			s.gui.flip_horizontally = true;
			s.gui.flip_horizontally = true;

			manager.set(assets::texture_id::ASSAULT_RIFLE, s);
		}

		{
			image_usage_settings s;
			s.gui.flip_horizontally = true;
			s.gui.flip_horizontally = false;

			manager.set(assets::texture_id::BILMER2000, s);
		}

		{
			image_usage_settings s;
			s.gui.flip_horizontally = true;
			s.gui.flip_horizontally = false;

			manager.set(assets::texture_id::KEK9, s);
		}

		{
			image_usage_settings s;
			s.gui.flip_horizontally = true;
			s.gui.flip_horizontally = true;

			manager.set(assets::texture_id::SHOTGUN, s);
		}

		{
			image_usage_settings s;
			s.gui.flip_horizontally = true;
			s.gui.flip_horizontally = true;

			manager.set(assets::texture_id::SUBMACHINE, s);
		}

		{
			image_usage_settings s;
			s.gui.flip_horizontally = true;
			s.gui.flip_horizontally = true;

			manager.set(assets::texture_id::PISTOL, s);
		}

		{
			image_usage_settings s;
			s.gui.flip_horizontally = true;
			s.gui.flip_horizontally = true;

			manager.set(assets::texture_id::URBAN_CYAN_MACHETE, s);
		}

		{
			image_usage_settings s;
			s.gui.flip_horizontally = true;
			s.gui.flip_horizontally = false;

			manager.set(assets::texture_id::AMPLIFIER_ARM, s);
		}

		{
			image_usage_settings s;
			s.gui.flip_horizontally = true;

			manager.set(assets::texture_id::SAMPLE_SUPPRESSOR, s);
		}

		{
			image_usage_settings s;
			s.gui.flip_horizontally = true;

			manager.set(assets::texture_id::SAMPLE_SUPPRESSOR, s);
		}

		{
			image_usage_settings s;
			s.gui.bbox_expander = vec2(0, 2);

			manager.set(assets::texture_id::BACKPACK, s);
		}

		manager.associate_neon_map(assets::texture_id::FORCE_GRENADE_RELEASED, assets::texture_id::FORCE_GRENADE);
		manager.associate_neon_map(assets::texture_id::PED_GRENADE_RELEASED, assets::texture_id::PED_GRENADE);
		manager.associate_neon_map(assets::texture_id::INTERFERENCE_GRENADE_RELEASED, assets::texture_id::INTERFERENCE_GRENADE);
	}
}