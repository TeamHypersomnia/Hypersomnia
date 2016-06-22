#pragma once
#include "all.h"
#include "game/resources/manager.h"
#include "graphics/shader.h"

namespace resource_setups {
	void load_standard_atlas() {
		resource_manager.create(assets::texture_id::TEST_CROSSHAIR, std::wstring(L"hypersomnia/gfx/crosshair.png"));
		resource_manager.create(assets::texture_id::TEST_PLAYER, L"hypersomnia/gfx/walk_1.png");
		resource_manager.create(assets::texture_id::TEST_BACKGROUND, L"hypersomnia/gfx/snow_textures/snow3.png");
		resource_manager.create(assets::texture_id::CRATE, L"hypersomnia/gfx/crate2.png");
		resource_manager.create(assets::texture_id::CRATE_DESTROYED, L"hypersomnia/gfx/crate2_destroyed.png");
		resource_manager.create(assets::texture_id::CAR_INSIDE, L"hypersomnia/gfx/crate2.png");
		resource_manager.create(assets::texture_id::CAR_FRONT, L"hypersomnia/gfx/crate2.png");

		resource_manager.create(assets::texture_id::MOTORCYCLE_FRONT, L"hypersomnia/gfx/motorcycle_front.png");
		resource_manager.create(assets::texture_id::MOTORCYCLE_INSIDE, L"hypersomnia/gfx/motorcycle_inside.png");

		resource_manager.create(assets::texture_id::TRUCK_FRONT, L"hypersomnia/gfx/truck_front.png");
		resource_manager.create(assets::texture_id::TRUCK_INSIDE, L"hypersomnia/gfx/truck_inside.png");

		resource_manager.create(assets::texture_id::TEST_SPRITE, L"hypersomnia/gfx/frog.png");

		{
			auto& gui = resource_manager.create(assets::texture_id::ASSAULT_RIFLE, L"hypersomnia/gfx/assault_rifle.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = true;
		}

		{
			auto& gui = resource_manager.create(assets::texture_id::SHOTGUN, L"hypersomnia/gfx/shotgun.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = true;
		}

		{
			auto& gui = resource_manager.create(assets::texture_id::SUBMACHINE, L"hypersomnia/gfx/submachine.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = true;
		}

		{
			auto& gui = resource_manager.create(assets::texture_id::PISTOL, L"hypersomnia/gfx/pistol.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = true;
		}

		{
			auto& gui = resource_manager.create(assets::texture_id::URBAN_CYAN_MACHETE, L"hypersomnia/gfx/urban_cyan_machete.png").gui_sprite_def;
			gui.flip_horizontally = true;
			gui.flip_vertically = true;
		}

		auto& magazine_gui = resource_manager.create(assets::texture_id::SAMPLE_MAGAZINE, L"hypersomnia/gfx/magazine.png").gui_sprite_def;
		magazine_gui.rotation_offset = -270;

		auto& suppressor_gui = resource_manager.create(assets::texture_id::SAMPLE_SUPPRESSOR, L"hypersomnia/gfx/suppressor.png").gui_sprite_def;
		suppressor_gui.flip_horizontally = true;

		resource_manager.create(assets::texture_id::ROUND_TRACE, L"hypersomnia/gfx/round_trace.png");
		resource_manager.create(assets::texture_id::PINK_CHARGE, L"hypersomnia/gfx/pink_charge.png");
		resource_manager.create(assets::texture_id::PINK_SHELL, L"hypersomnia/gfx/pink_shell.png");
		resource_manager.create(assets::texture_id::CYAN_CHARGE, L"hypersomnia/gfx/cyan_charge.png");
		resource_manager.create(assets::texture_id::CYAN_SHELL, L"hypersomnia/gfx/cyan_shell.png");
		resource_manager.create(assets::texture_id::GREEN_CHARGE, L"hypersomnia/gfx/green_charge.png");
		resource_manager.create(assets::texture_id::GREEN_SHELL, L"hypersomnia/gfx/green_shell.png");

		auto& backpack_gui = resource_manager.create(assets::texture_id::BACKPACK, L"hypersomnia/gfx/backpack.png").gui_sprite_def;
		backpack_gui.rotation_offset = -90.f;
		backpack_gui.gui_bbox_expander = vec2(0, 2);

		augs::image attachment_circle_filled;
		attachment_circle_filled.paint_filled_circle(16);

		augs::image attachment_circle_border;
		attachment_circle_border.paint_circle_midpoint(16);

		resource_manager.create(assets::texture_id::ATTACHMENT_CIRCLE_FILLED, attachment_circle_filled);
		resource_manager.create(assets::texture_id::ATTACHMENT_CIRCLE_BORDER, attachment_circle_border);

		augs::image hud_circular_hud_medium;
		//hud_circular_hud_medium.paint_circle(60, 10, augs::white, true);
		//hud_circular_hud_medium.paint_circle_midpoint(58, augs::rgba(0, 0, 0, 0));
		hud_circular_hud_medium.paint_circle_midpoint(67, 1, augs::cyan, false, false, vec2().set_from_degrees(-45), vec2().set_from_degrees(45));
		hud_circular_hud_medium.paint_circle_midpoint(65, 5, augs::red, false, false, vec2().set_from_degrees(135), vec2().set_from_degrees(-180 + 30));
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

		//hud_circular_hud_medium.save(L"saved.png");
		//hud_circular_hud_medium.paint_circle(48+5, 5);
		//hud_circular_hud_medium.paint_circle_midpoint(40);

		resource_manager.create(assets::texture_id::HUD_CIRCULAR_BAR_MEDIUM, hud_circular_hud_medium);

		resource_manager.create(assets::texture_id::CONTAINER_OPEN_ICON, L"hypersomnia/gfx/container_open_icon.png");
		resource_manager.create(assets::texture_id::CONTAINER_CLOSED_ICON, L"hypersomnia/gfx/container_closed_icon.png");

		resource_manager.create(assets::texture_id::GUI_CURSOR, L"hypersomnia/gfx/gui_cursor.png");
		resource_manager.create(assets::texture_id::GUI_CURSOR_ADD, L"hypersomnia/gfx/gui_cursor_add.png");
		resource_manager.create(assets::texture_id::GUI_CURSOR_ERROR, L"hypersomnia/gfx/gui_cursor_error.png");
		resource_manager.create(assets::texture_id::GUI_CURSOR_MINUS, L"hypersomnia/gfx/gui_cursor_minus.png");

		resource_manager.create(assets::texture_id::BLANK, L"hypersomnia/gfx/blank.png");
		resource_manager.create(assets::texture_id::DROP_HAND_ICON, L"hypersomnia/gfx/drop_hand_icon.png");
		resource_manager.create(assets::texture_id::SECONDARY_HAND_ICON, L"hypersomnia/gfx/secondary_hand_icon.png");
		resource_manager.create(assets::texture_id::PRIMARY_HAND_ICON, L"hypersomnia/gfx/primary_hand_icon.png");
		resource_manager.create(assets::texture_id::SHOULDER_SLOT_ICON, L"hypersomnia/gfx/shoulder_slot_icon.png");
		resource_manager.create(assets::texture_id::ARMOR_SLOT_ICON, L"hypersomnia/gfx/armor_slot_icon.png");
		resource_manager.create(assets::texture_id::CHAMBER_SLOT_ICON, L"hypersomnia/gfx/chamber_slot_icon.png");
		resource_manager.create(assets::texture_id::DETACHABLE_MAGAZINE_ICON, L"hypersomnia/gfx/detachable_magazine_slot_icon.png");
		resource_manager.create(assets::texture_id::GUN_BARREL_SLOT_ICON, L"hypersomnia/gfx/gun_barrel_slot_icon.png");
		
		resource_manager.create(assets::texture_id::DEAD_TORSO, L"hypersomnia/gfx/dead_torso.png");

		auto& font = resource_manager.create(assets::font_id::GUI_FONT);
		font.open("hypersomnia/Kubasta.ttf", 16, L" ABCDEFGHIJKLMNOPRSTUVWXYZQabcdefghijklmnoprstuvwxyzq0123456789.!@#$%^&*()_+-=[];'\\,./{}:\"|<>?");

		resource_manager.create_sprites_indexed(
			assets::texture_id::TORSO_MOVING_FIRST,
			assets::texture_id::TORSO_MOVING_LAST,
			L"hypersomnia/gfx/sprite");

		resource_manager.create_sprites_indexed(
			assets::texture_id::SMOKE_PARTICLE_FIRST,
			assets::texture_id::SMOKE_PARTICLE_LAST,
			L"hypersomnia/gfx/smoke");

		resource_manager.create_sprites_indexed(
			assets::texture_id::PIXEL_THUNDER_FIRST,
			assets::texture_id::PIXEL_THUNDER_LAST,
			L"hypersomnia/gfx/pixel_thunder");

		resource_manager.create(assets::atlas_id::GAME_WORLD_ATLAS,
			resources::manager::atlas_creation_mode::FROM_ALL_TEXTURES
			| resources::manager::atlas_creation_mode::FROM_ALL_FONTS);

		resource_manager.create_inverse_with_flip(assets::animation_id::TORSO_MOVE,
			assets::texture_id::TORSO_MOVING_FIRST,
			assets::texture_id::TORSO_MOVING_LAST,
			20.0f);

		auto& player_response = resource_manager.create(assets::animation_response_id::TORSO_SET);
		player_response[animation_response_type::MOVE] = assets::animation_id::TORSO_MOVE;
	}
}