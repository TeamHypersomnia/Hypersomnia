#include "all.h"
#include "game/resources/manager.h"
#include "augs/graphics/shader.h"

namespace resource_setups {
	void load_standard_atlas() {
		auto& manager = get_resource_manager();

		manager.create(assets::texture_id::TEST_CROSSHAIR, "hypersomnia/gfx/crosshair.png");
		manager.create(assets::texture_id::TEST_PLAYER, "hypersomnia/gfx/walk_1.png");
		manager.create(assets::texture_id::TEST_BACKGROUND, "hypersomnia/gfx/snow_textures/snow3.png");
		manager.create(assets::texture_id::CRATE, "hypersomnia/gfx/crate2.png");
		manager.create(assets::texture_id::CRATE_DESTROYED, "hypersomnia/gfx/crate2_destroyed.png");
		manager.create(assets::texture_id::CAR_INSIDE, "hypersomnia/gfx/crate2.png");
		manager.create(assets::texture_id::CAR_FRONT, "hypersomnia/gfx/crate2.png");

		manager.create(assets::texture_id::JMIX114, "hypersomnia/gfx/jmix114.png");

		manager.create(assets::texture_id::TRUCK_FRONT, "hypersomnia/gfx/truck_front.png");
		manager.create(assets::texture_id::TRUCK_INSIDE, "hypersomnia/gfx/truck_inside.png");

		manager.create(assets::texture_id::MENU_GAME_LOGO, "hypersomnia/gfx/menu_game_logo.png");

		{
			auto& gui = manager.create(assets::texture_id::ASSAULT_RIFLE, "hypersomnia/gfx/assault_rifle.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = true;
		}

		{
			auto& gui = manager.create(assets::texture_id::BILMER2000, "hypersomnia/gfx/bilmer2000.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = false;
		}

		{
			auto& gui = manager.create(assets::texture_id::KEK9, "hypersomnia/gfx/kek9.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = false;
		}

		{
			auto& gui = manager.create(assets::texture_id::SHOTGUN, "hypersomnia/gfx/shotgun.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = true;
		}

		{
			auto& gui = manager.create(assets::texture_id::SUBMACHINE, "hypersomnia/gfx/submachine.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = true;
		}

		{
			auto& gui = manager.create(assets::texture_id::PISTOL, "hypersomnia/gfx/pistol.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = true;
		}

		{
			auto& gui = manager.create(assets::texture_id::URBAN_CYAN_MACHETE, "hypersomnia/gfx/urban_cyan_machete.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = true;
		}

		auto& magazine_gui = manager.create(assets::texture_id::SAMPLE_MAGAZINE, "hypersomnia/gfx/magazine.png").gui_sprite_def;

		manager.create(assets::texture_id::SMALL_MAGAZINE, "hypersomnia/gfx/small_magazine.png");


		auto& suppressor_gui = manager.create(assets::texture_id::SAMPLE_SUPPRESSOR, "hypersomnia/gfx/suppressor.png").gui_sprite_def;
		suppressor_gui.flip_horizontally = true;

		manager.create(assets::texture_id::ROUND_TRACE, "hypersomnia/gfx/round_trace.png");
		manager.create(assets::texture_id::PINK_CHARGE, "hypersomnia/gfx/pink_charge.png");
		manager.create(assets::texture_id::PINK_SHELL, "hypersomnia/gfx/pink_shell.png");
		manager.create(assets::texture_id::CYAN_CHARGE, "hypersomnia/gfx/cyan_charge.png");
		manager.create(assets::texture_id::CYAN_SHELL, "hypersomnia/gfx/cyan_shell.png");
		manager.create(assets::texture_id::GREEN_CHARGE, "hypersomnia/gfx/green_charge.png");
		manager.create(assets::texture_id::GREEN_SHELL, "hypersomnia/gfx/green_shell.png");

		auto& backpack_gui = manager.create(assets::texture_id::BACKPACK, "hypersomnia/gfx/backpack.png").gui_sprite_def;
		backpack_gui.gui_bbox_expander = vec2(0, 2);

		augs::image attachment_circle_filled;
		attachment_circle_filled.paint_filled_circle(16);

		augs::image attachment_circle_border;
		attachment_circle_border.paint_circle_midpoint(16);

		manager.create(assets::texture_id::ATTACHMENT_CIRCLE_FILLED, attachment_circle_filled);
		manager.create(assets::texture_id::ATTACHMENT_CIRCLE_BORDER, attachment_circle_border);

		augs::image hud_circular_hud_medium;
		//hud_circular_hud_medium.paint_circle(60, 10, white, true);
		//hud_circular_hud_medium.paint_circle_midpoint(58, rgba(0, 0, 0, 0));
		hud_circular_hud_medium.paint_circle_midpoint(57, 1, cyan, false, false, vec2().set_from_degrees(-45), vec2().set_from_degrees(45));
		hud_circular_hud_medium.paint_circle_midpoint(55, 5, white, false, false, vec2().set_from_degrees(135), vec2().set_from_degrees(-180 + 30));
		//hud_circular_hud_medium.paint_circle_midpoint(59, 1, cyan, false, false, vec2().set_from_degrees(-45), vec2().set_from_degrees(45));
		//hud_circular_hud_medium.paint_circle_midpoint(68, 1, white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(67, 1, white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(66, 1, white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(65, 1, white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(64, 1, white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(63, 1, white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(62, 1, white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(61, 1, white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(60, 1, white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(59, 1, white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(58, 1, white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(56, 1, white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(55, 1, white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(54, 1, white, true, -45, 45, false);
//		hud_circular_hud_medium.paint_circle_midpoint(53, 1, white, true, -45, 45, false);
//		hud_circular_hud_medium.paint_circle_midpoint(52, 1, white, true, -45, 45, false);
//		hud_circular_hud_medium.paint_circle_midpoint(51, 1, white, true, -45, 45, false);
//		hud_circular_hud_medium.paint_circle_midpoint(50, 1, white, true, -45, 45, false);
//		hud_circular_hud_medium.paint_circle_midpoint(48, 1, white, true, -45, 45, false);

		//hud_circular_hud_medium.save("saved.png");
		//hud_circular_hud_medium.paint_circle(48+5, 5);
		//hud_circular_hud_medium.paint_circle_midpoint(40);

		manager.create(assets::texture_id::HUD_CIRCULAR_BAR_MEDIUM, hud_circular_hud_medium);

		make_button_with_corners(
			manager,
			assets::texture_id::HOTBAR_BUTTON_INSIDE,
			white,
			white,
			20,
			8,
			4,
			true
		);

		make_button_with_corners(
			manager,
			assets::texture_id::MENU_BUTTON_INSIDE,
			white,
			white,
			12,
			8,
			4,
			false
		);

		manager.create(assets::texture_id::CONTAINER_OPEN_ICON, "hypersomnia/gfx/container_open_icon.png");
		manager.create(assets::texture_id::CONTAINER_CLOSED_ICON, "hypersomnia/gfx/container_closed_icon.png");

		manager.create(assets::texture_id::GUI_CURSOR, "hypersomnia/gfx/gui_cursor.png");
		manager.create(assets::texture_id::GUI_CURSOR_HOVER, "hypersomnia/gfx/gui_cursor_hover.png");
		manager.create(assets::texture_id::GUI_CURSOR_ADD, "hypersomnia/gfx/gui_cursor_add.png");
		manager.create(assets::texture_id::GUI_CURSOR_ERROR, "hypersomnia/gfx/gui_cursor_error.png");
		manager.create(assets::texture_id::GUI_CURSOR_MINUS, "hypersomnia/gfx/gui_cursor_minus.png");

		manager.create(assets::texture_id::BLANK, "hypersomnia/gfx/blank.png");
		manager.create(assets::texture_id::LASER, "hypersomnia/gfx/laser.png");
		manager.create(assets::texture_id::LASER_GLOW_EDGE, "hypersomnia/gfx/laser_glow_edge.png");
		manager.create(assets::texture_id::DROP_HAND_ICON, "hypersomnia/gfx/drop_hand_icon.png");
		manager.create(assets::texture_id::SECONDARY_HAND_ICON, "hypersomnia/gfx/secondary_hand_icon.png");
		manager.create(assets::texture_id::PRIMARY_HAND_ICON, "hypersomnia/gfx/primary_hand_icon.png");
		manager.create(assets::texture_id::SHOULDER_SLOT_ICON, "hypersomnia/gfx/shoulder_slot_icon.png");
		manager.create(assets::texture_id::ARMOR_SLOT_ICON, "hypersomnia/gfx/armor_slot_icon.png");
		manager.create(assets::texture_id::CHAMBER_SLOT_ICON, "hypersomnia/gfx/chamber_slot_icon.png");
		manager.create(assets::texture_id::DETACHABLE_MAGAZINE_ICON, "hypersomnia/gfx/detachable_magazine_slot_icon.png");
		manager.create(assets::texture_id::GUN_MUZZLE_SLOT_ICON, "hypersomnia/gfx/gun_muzzle_slot_icon.png");
		
		manager.create(assets::texture_id::DEAD_TORSO, "hypersomnia/gfx/dead_torso.png");

		auto& font = manager.create(assets::font_id::GUI_FONT);
		font.open("hypersomnia/Kubasta.ttf", 16, L" ABCDEFGHIJKLMNOPRSTUVWXYZQabcdefghijklmnoprstuvwxyzq0123456789.!@#$%^&*()_+-=[];'\\,./{}:\"|<>?~");

		manager.create_sprites_indexed(
			assets::texture_id::TORSO_MOVING_FIRST,
			assets::texture_id::TORSO_MOVING_LAST,
			"hypersomnia/gfx/torso_white_walk_barehands");

		manager.create_sprites_indexed(
			assets::texture_id::VIOLET_TORSO_MOVING_FIRST,
			assets::texture_id::VIOLET_TORSO_MOVING_LAST,
			"hypersomnia/gfx/sprite");

		manager.create_sprites_indexed(
			assets::texture_id::BLUE_TORSO_MOVING_FIRST,
			assets::texture_id::BLUE_TORSO_MOVING_LAST,
			"hypersomnia/gfx/walk");

		manager.create_sprites_indexed(
			assets::texture_id::METROPOLIS_TILE_FIRST,
			assets::texture_id::METROPOLIS_TILE_LAST,
			"hypersomnia/gfx/tileset/tile");

		manager.create_sprites_indexed(
			assets::texture_id::SMOKE_PARTICLE_FIRST,
			assets::texture_id(int(assets::texture_id::SMOKE_PARTICLE_FIRST) + 3),
			"hypersomnia/gfx/smoke");

		manager.create_sprites_indexed(
			assets::texture_id::PIXEL_THUNDER_FIRST,
			assets::texture_id::PIXEL_THUNDER_LAST,
			"hypersomnia/gfx/pixel_thunder");

		manager.create_sprites_indexed(
			assets::texture_id::BLINK_FIRST,
			assets::texture_id::BLINK_LAST,
			"hypersomnia/gfx/blink");

		manager.create(assets::texture_id::HAVE_A_PLEASANT, "hypersomnia/gfx/have_a_pleasant.png");
		manager.create(assets::texture_id::AWAKENING, "hypersomnia/gfx/awakening.png");
		manager.create(assets::texture_id::METROPOLIS, "hypersomnia/gfx/metropolis.png");

		manager.create(assets::texture_id::BRICK_WALL, "hypersomnia/gfx/brick_wall.png");
		manager.create(assets::texture_id::ROAD, "hypersomnia/gfx/road.png");
		manager.create(assets::texture_id::ROAD_FRONT_DIRT, "hypersomnia/gfx/road_front_dirt.png");

		manager.create(assets::texture_id::WANDERING_CROSS, "hypersomnia/gfx/wandering_cross.png");

		manager.create(assets::texture_id::TRUCK_ENGINE, "hypersomnia/gfx/truck_engine.png");

		manager.create_inverse_with_flip(assets::animation_id::TORSO_MOVE,
			assets::texture_id::TORSO_MOVING_FIRST,
			assets::texture_id::TORSO_MOVING_LAST,
			20.0f);

		manager.create_inverse_with_flip(assets::animation_id::BLUE_TORSO_MOVE,
			assets::texture_id::BLUE_TORSO_MOVING_FIRST,
			assets::texture_id::BLUE_TORSO_MOVING_LAST,
			20.0f);

		manager.create_inverse_with_flip(assets::animation_id::VIOLET_TORSO_MOVE,
			assets::texture_id::VIOLET_TORSO_MOVING_FIRST,
			assets::texture_id::VIOLET_TORSO_MOVING_LAST,
			20.0f);

		manager.create(assets::animation_id::BLINK_ANIMATION,
			assets::texture_id::BLINK_FIRST,
			assets::texture_id::BLINK_LAST,
			50.0f, resources::animation::loop_type::NONE);

		{
			auto& player_response = manager.create(assets::animation_response_id::TORSO_SET);
			player_response[animation_response_type::MOVE] = assets::animation_id::TORSO_MOVE;
		}

		{
			auto& player_response = manager.create(assets::animation_response_id::BLUE_TORSO_SET);
			player_response[animation_response_type::MOVE] = assets::animation_id::BLUE_TORSO_MOVE;
		}

		{
			auto& player_response = manager.create(assets::animation_response_id::VIOLET_TORSO_SET);
			player_response[animation_response_type::MOVE] = assets::animation_id::VIOLET_TORSO_MOVE;
		}
	}
}