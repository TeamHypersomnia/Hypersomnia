#include "all.h"
#include "game/resources/manager.h"
#include "augs/graphics/shader.h"

namespace resource_setups {
	void load_standard_atlas() {
		get_resource_manager().create(assets::texture_id::TEST_CROSSHAIR, "hypersomnia/gfx/crosshair.png");
		get_resource_manager().create(assets::texture_id::TEST_PLAYER, "hypersomnia/gfx/walk_1.png");
		get_resource_manager().create(assets::texture_id::TEST_BACKGROUND, "hypersomnia/gfx/snow_textures/snow3.png");
		get_resource_manager().create(assets::texture_id::CRATE, "hypersomnia/gfx/crate2.png");
		get_resource_manager().create(assets::texture_id::CRATE_DESTROYED, "hypersomnia/gfx/crate2_destroyed.png");
		get_resource_manager().create(assets::texture_id::CAR_INSIDE, "hypersomnia/gfx/crate2.png");
		get_resource_manager().create(assets::texture_id::CAR_FRONT, "hypersomnia/gfx/crate2.png");

		get_resource_manager().create(assets::texture_id::MOTORCYCLE_FRONT, "hypersomnia/gfx/motorcycle_front.png");
		get_resource_manager().create(assets::texture_id::MOTORCYCLE_INSIDE, "hypersomnia/gfx/motorcycle_inside.png");

		get_resource_manager().create(assets::texture_id::TRUCK_FRONT, "hypersomnia/gfx/truck_front.png");
		get_resource_manager().create(assets::texture_id::TRUCK_INSIDE, "hypersomnia/gfx/truck_inside.png");

		get_resource_manager().create(assets::texture_id::TEST_SPRITE, "hypersomnia/gfx/frog.png");

		{
			auto& gui = get_resource_manager().create(assets::texture_id::ASSAULT_RIFLE, "hypersomnia/gfx/assault_rifle.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = true;
		}

		{
			auto& gui = get_resource_manager().create(assets::texture_id::BILMER2000, "hypersomnia/gfx/bilmer2000.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = false;
		}

		{
			auto& gui = get_resource_manager().create(assets::texture_id::KEK9, "hypersomnia/gfx/kek9.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = false;
		}

		{
			auto& gui = get_resource_manager().create(assets::texture_id::SHOTGUN, "hypersomnia/gfx/shotgun.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = true;
		}

		{
			auto& gui = get_resource_manager().create(assets::texture_id::SUBMACHINE, "hypersomnia/gfx/submachine.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = true;
		}

		{
			auto& gui = get_resource_manager().create(assets::texture_id::PISTOL, "hypersomnia/gfx/pistol.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = true;
		}

		{
			auto& gui = get_resource_manager().create(assets::texture_id::URBAN_CYAN_MACHETE, "hypersomnia/gfx/urban_cyan_machete.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = true;
		}

		auto& magazine_gui = get_resource_manager().create(assets::texture_id::SAMPLE_MAGAZINE, "hypersomnia/gfx/magazine.png").gui_sprite_def;
		magazine_gui.rotation_offset = -270;

		get_resource_manager().create(assets::texture_id::SMALL_MAGAZINE, "hypersomnia/gfx/small_magazine.png");


		auto& suppressor_gui = get_resource_manager().create(assets::texture_id::SAMPLE_SUPPRESSOR, "hypersomnia/gfx/suppressor.png").gui_sprite_def;
		suppressor_gui.flip_horizontally = true;

		get_resource_manager().create(assets::texture_id::ROUND_TRACE, "hypersomnia/gfx/round_trace.png");
		get_resource_manager().create(assets::texture_id::PINK_CHARGE, "hypersomnia/gfx/pink_charge.png");
		get_resource_manager().create(assets::texture_id::PINK_SHELL, "hypersomnia/gfx/pink_shell.png");
		get_resource_manager().create(assets::texture_id::CYAN_CHARGE, "hypersomnia/gfx/cyan_charge.png");
		get_resource_manager().create(assets::texture_id::CYAN_SHELL, "hypersomnia/gfx/cyan_shell.png");
		get_resource_manager().create(assets::texture_id::GREEN_CHARGE, "hypersomnia/gfx/green_charge.png");
		get_resource_manager().create(assets::texture_id::GREEN_SHELL, "hypersomnia/gfx/green_shell.png");

		auto& backpack_gui = get_resource_manager().create(assets::texture_id::BACKPACK, "hypersomnia/gfx/backpack.png").gui_sprite_def;
		backpack_gui.rotation_offset = -90.f;
		backpack_gui.gui_bbox_expander = vec2(0, 2);

		augs::image attachment_circle_filled;
		attachment_circle_filled.paint_filled_circle(16);

		augs::image attachment_circle_border;
		attachment_circle_border.paint_circle_midpoint(16);

		get_resource_manager().create(assets::texture_id::ATTACHMENT_CIRCLE_FILLED, attachment_circle_filled);
		get_resource_manager().create(assets::texture_id::ATTACHMENT_CIRCLE_BORDER, attachment_circle_border);

		augs::image hud_circular_hud_medium;
		//hud_circular_hud_medium.paint_circle(60, 10, augs::white, true);
		//hud_circular_hud_medium.paint_circle_midpoint(58, augs::rgba(0, 0, 0, 0));
		hud_circular_hud_medium.paint_circle_midpoint(57, 1, augs::cyan, false, false, vec2().set_from_degrees(-45), vec2().set_from_degrees(45));
		hud_circular_hud_medium.paint_circle_midpoint(55, 5, augs::white, false, false, vec2().set_from_degrees(135), vec2().set_from_degrees(-180 + 30));
		//hud_circular_hud_medium.paint_circle_midpoint(59, 1, augs::cyan, false, false, vec2().set_from_degrees(-45), vec2().set_from_degrees(45));
		//hud_circular_hud_medium.paint_circle_midpoint(68, 1, augs::white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(67, 1, augs::white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(66, 1, augs::white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(65, 1, augs::white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(64, 1, augs::white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(63, 1, augs::white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(62, 1, augs::white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(61, 1, augs::white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(60, 1, augs::white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(59, 1, augs::white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(58, 1, augs::white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(56, 1, augs::white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(55, 1, augs::white, true, -45, 45, false);
		//hud_circular_hud_medium.paint_circle_midpoint(54, 1, augs::white, true, -45, 45, false);
//		hud_circular_hud_medium.paint_circle_midpoint(53, 1, augs::white, true, -45, 45, false);
//		hud_circular_hud_medium.paint_circle_midpoint(52, 1, augs::white, true, -45, 45, false);
//		hud_circular_hud_medium.paint_circle_midpoint(51, 1, augs::white, true, -45, 45, false);
//		hud_circular_hud_medium.paint_circle_midpoint(50, 1, augs::white, true, -45, 45, false);
//		hud_circular_hud_medium.paint_circle_midpoint(48, 1, augs::white, true, -45, 45, false);

		//hud_circular_hud_medium.save("saved.png");
		//hud_circular_hud_medium.paint_circle(48+5, 5);
		//hud_circular_hud_medium.paint_circle_midpoint(40);

		get_resource_manager().create(assets::texture_id::HUD_CIRCULAR_BAR_MEDIUM, hud_circular_hud_medium);

		get_resource_manager().create(assets::texture_id::CONTAINER_OPEN_ICON, "hypersomnia/gfx/container_open_icon.png");
		get_resource_manager().create(assets::texture_id::CONTAINER_CLOSED_ICON, "hypersomnia/gfx/container_closed_icon.png");

		get_resource_manager().create(assets::texture_id::GUI_CURSOR, "hypersomnia/gfx/gui_cursor.png");
		get_resource_manager().create(assets::texture_id::GUI_CURSOR_ADD, "hypersomnia/gfx/gui_cursor_add.png");
		get_resource_manager().create(assets::texture_id::GUI_CURSOR_ERROR, "hypersomnia/gfx/gui_cursor_error.png");
		get_resource_manager().create(assets::texture_id::GUI_CURSOR_MINUS, "hypersomnia/gfx/gui_cursor_minus.png");

		get_resource_manager().create(assets::texture_id::BLANK, "hypersomnia/gfx/blank.png");
		get_resource_manager().create(assets::texture_id::DROP_HAND_ICON, "hypersomnia/gfx/drop_hand_icon.png");
		get_resource_manager().create(assets::texture_id::SECONDARY_HAND_ICON, "hypersomnia/gfx/secondary_hand_icon.png");
		get_resource_manager().create(assets::texture_id::PRIMARY_HAND_ICON, "hypersomnia/gfx/primary_hand_icon.png");
		get_resource_manager().create(assets::texture_id::SHOULDER_SLOT_ICON, "hypersomnia/gfx/shoulder_slot_icon.png");
		get_resource_manager().create(assets::texture_id::ARMOR_SLOT_ICON, "hypersomnia/gfx/armor_slot_icon.png");
		get_resource_manager().create(assets::texture_id::CHAMBER_SLOT_ICON, "hypersomnia/gfx/chamber_slot_icon.png");
		get_resource_manager().create(assets::texture_id::DETACHABLE_MAGAZINE_ICON, "hypersomnia/gfx/detachable_magazine_slot_icon.png");
		get_resource_manager().create(assets::texture_id::GUN_BARREL_SLOT_ICON, "hypersomnia/gfx/gun_barrel_slot_icon.png");
		
		get_resource_manager().create(assets::texture_id::DEAD_TORSO, "hypersomnia/gfx/dead_torso.png");

		auto& font = get_resource_manager().create(assets::font_id::GUI_FONT);
		font.open("hypersomnia/Kubasta.ttf", 16, L" ABCDEFGHIJKLMNOPRSTUVWXYZQabcdefghijklmnoprstuvwxyzq0123456789.!@#$%^&*()_+-=[];'\\,./{}:\"|<>?");

		get_resource_manager().create_sprites_indexed(
			assets::texture_id::TORSO_MOVING_FIRST,
			assets::texture_id::TORSO_MOVING_LAST,
			"hypersomnia/gfx/torso_white_walk_barehands");

		get_resource_manager().create_sprites_indexed(
			assets::texture_id::VIOLET_TORSO_MOVING_FIRST,
			assets::texture_id::VIOLET_TORSO_MOVING_LAST,
			"hypersomnia/gfx/sprite");

		get_resource_manager().create_sprites_indexed(
			assets::texture_id::BLUE_TORSO_MOVING_FIRST,
			assets::texture_id::BLUE_TORSO_MOVING_LAST,
			"hypersomnia/gfx/walk");

		get_resource_manager().create_sprites_indexed(
			assets::texture_id::METROPOLIS_TILE_FIRST,
			assets::texture_id::METROPOLIS_TILE_LAST,
			"hypersomnia/gfx/tileset/tile");

		get_resource_manager().create_sprites_indexed(
			assets::texture_id::SMOKE_PARTICLE_FIRST,
			assets::texture_id(int(assets::texture_id::SMOKE_PARTICLE_FIRST) + 3),
			"hypersomnia/gfx/smoke");

		get_resource_manager().create_sprites_indexed(
			assets::texture_id::PIXEL_THUNDER_FIRST,
			assets::texture_id::PIXEL_THUNDER_LAST,
			"hypersomnia/gfx/pixel_thunder");

		get_resource_manager().create_sprites_indexed(
			assets::texture_id::BLINK_FIRST,
			assets::texture_id::BLINK_LAST,
			"hypersomnia/gfx/blink");

		get_resource_manager().create(assets::texture_id::HAVE_A_PLEASANT, "hypersomnia/gfx/have_a_pleasant.png");
		get_resource_manager().create(assets::texture_id::AWAKENING, "hypersomnia/gfx/awakening.png");
		get_resource_manager().create(assets::texture_id::METROPOLIS, "hypersomnia/gfx/metropolis.png");

		get_resource_manager().create(assets::texture_id::BRICK_WALL, "hypersomnia/gfx/brick_wall.png");
		get_resource_manager().create(assets::texture_id::ROAD, "hypersomnia/gfx/road.png");
		get_resource_manager().create(assets::texture_id::ROAD_FRONT_DIRT, "hypersomnia/gfx/road_front_dirt.png");

		get_resource_manager().create(assets::texture_id::WANDERING_CROSS, "hypersomnia/gfx/wandering_cross.png");

		get_resource_manager().create_inverse_with_flip(assets::animation_id::TORSO_MOVE,
			assets::texture_id::TORSO_MOVING_FIRST,
			assets::texture_id::TORSO_MOVING_LAST,
			20.0f);

		get_resource_manager().create_inverse_with_flip(assets::animation_id::BLUE_TORSO_MOVE,
			assets::texture_id::BLUE_TORSO_MOVING_FIRST,
			assets::texture_id::BLUE_TORSO_MOVING_LAST,
			20.0f);

		get_resource_manager().create_inverse_with_flip(assets::animation_id::VIOLET_TORSO_MOVE,
			assets::texture_id::VIOLET_TORSO_MOVING_FIRST,
			assets::texture_id::VIOLET_TORSO_MOVING_LAST,
			20.0f);

		get_resource_manager().create(assets::animation_id::BLINK_ANIMATION,
			assets::texture_id::BLINK_FIRST,
			assets::texture_id::BLINK_LAST,
			50.0f, resources::animation::loop_type::NONE);

		{
			auto& player_response = get_resource_manager().create(assets::animation_response_id::TORSO_SET);
			player_response[animation_response_type::MOVE] = assets::animation_id::TORSO_MOVE;
		}

		{
			auto& player_response = get_resource_manager().create(assets::animation_response_id::BLUE_TORSO_SET);
			player_response[animation_response_type::MOVE] = assets::animation_id::BLUE_TORSO_MOVE;
		}

		{
			auto& player_response = get_resource_manager().create(assets::animation_response_id::VIOLET_TORSO_SET);
			player_response[animation_response_type::MOVE] = assets::animation_id::VIOLET_TORSO_MOVE;
		}
	}
}