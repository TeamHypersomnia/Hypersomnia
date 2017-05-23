#include "game/build_settings.h"
#if BUILD_TEST_SCENES
#include "all.h"
#include "game/assets/assets_manager.h"

void set_standard_sound_buffers(assets_manager& manager) {
	{
		auto& buf = manager[assets::sound_buffer_id::BILMER2000_MUZZLE];
		buf.from_file("resources/sfx/bilmer2000_muzzle.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::ASSAULT_RIFLE_MUZZLE];
		buf.from_file("resources/sfx/assault_muzzle.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::SUBMACHINE_MUZZLE];
		buf.from_file("resources/sfx/submachine_muzzle.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::KEK9_MUZZLE];
		buf.from_file("resources/sfx/kek9_muzzle.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::ROCKET_LAUNCHER_MUZZLE];
		buf.from_file("resources/sfx/rocket_launcher_muzzle.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::ELECTRIC_PROJECTILE_FLIGHT];
		buf.from_file("resources/sfx/electric_projectile_flight.wav");
	}
	
	{
		auto& buf = manager[assets::sound_buffer_id::MISSILE_THRUSTER];
		buf.from_file("resources/sfx/missile_thruster.wav");
	}
	
	{
		auto& buf = manager[assets::sound_buffer_id::BULLET_PASSES_THROUGH_HELD_ITEM];
		buf.from_file("resources/sfx/bullet_hits_held_item.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::IMPACT];
		buf.from_file("resources/sfx/impact_light_%x.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::DEATH];
		buf.from_file("resources/sfx/impact_%x.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::ENGINE];
		buf.from_file("resources/sfx/engine.ogg");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::ELECTRIC_DISCHARGE_EXPLOSION];
		buf.from_file("resources/sfx/electric_discharge_explosion_%x.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::WIND];
		buf.from_file("resources/sfx/wind_%x.ogg");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::BUTTON_HOVER];
		buf.from_file("resources/sfx/button_hover.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::BUTTON_CLICK];
		buf.from_file("resources/sfx/button_click.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::LOW_AMMO_CUE];
		buf.from_file("resources/sfx/low_ammo_cue.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::FIREARM_ENGINE];
		buf.from_file("resources/sfx/firearm_engine.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::CAST_SUCCESSFUL];
		buf.from_file("resources/sfx/cast_successful.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::CAST_UNSUCCESSFUL];
		buf.from_file("resources/sfx/cast_unsuccessful.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::EXPLOSION];
		buf.from_file("resources/sfx/explosion.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::GREAT_EXPLOSION];
		buf.from_file("resources/sfx/great_explosion.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::PED_EXPLOSION];
		buf.from_file("resources/sfx/ped_explosion.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::INTERFERENCE_EXPLOSION];
		buf.from_file("resources/sfx/interference_explosion.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::CAST_CHARGING];
		buf.from_file("resources/sfx/cast_charging.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::GRENADE_UNPIN];
		buf.from_file("resources/sfx/grenade_unpin.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::GRENADE_THROW];
		buf.from_file("resources/sfx/grenade_throw.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::ITEM_THROW];
		buf.from_file("resources/sfx/item_throw.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::COLLISION_METAL_METAL];
		buf.from_file("resources/sfx/collision_metal_metal_%x.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::COLLISION_METAL_WOOD];
		buf.from_file("resources/sfx/collision_metal_wood_%x.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::COLLISION_GRENADE];
		buf.from_file("resources/sfx/collision_grenade.wav");
	}
}
#endif