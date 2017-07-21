#include "game/hardcoded_content/all_hardcoded_content.h"
#include "game/assets/assets_manager.h"

void load_test_scene_sound_buffers(assets_manager& manager) {
	{
		auto& buf = manager[assets::sound_buffer_id::BILMER2000_MUZZLE];
		buf.from_file("content/official/sfx/bilmer2000_muzzle.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::ASSAULT_RIFLE_MUZZLE];
		buf.from_file("content/official/sfx/assault_muzzle.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::SUBMACHINE_MUZZLE];
		buf.from_file("content/official/sfx/submachine_muzzle.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::KEK9_MUZZLE];
		buf.from_file("content/official/sfx/kek9_muzzle.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::ROCKET_LAUNCHER_MUZZLE];
		buf.from_file("content/official/sfx/rocket_launcher_muzzle.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::ELECTRIC_PROJECTILE_FLIGHT];
		buf.from_file("content/official/sfx/electric_projectile_flight.wav");
	}
	
	{
		auto& buf = manager[assets::sound_buffer_id::MISSILE_THRUSTER];
		buf.from_file("content/official/sfx/missile_thruster.wav");
	}
	
	{
		auto& buf = manager[assets::sound_buffer_id::BULLET_PASSES_THROUGH_HELD_ITEM];
		buf.from_file("content/official/sfx/bullet_hits_held_item.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::IMPACT];
		buf.from_file("content/official/sfx/impact_light_%x.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::DEATH];
		buf.from_file("content/official/sfx/impact_%x.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::ENGINE];
		buf.from_file("content/official/sfx/engine.ogg");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::ELECTRIC_DISCHARGE_EXPLOSION];
		buf.from_file("content/official/sfx/electric_discharge_explosion_%x.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::WIND];
		buf.from_file("content/official/sfx/wind_%x.ogg");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::LOW_AMMO_CUE];
		buf.from_file("content/official/sfx/low_ammo_cue.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::FIREARM_ENGINE];
		buf.from_file("content/official/sfx/firearm_engine.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::CAST_SUCCESSFUL];
		buf.from_file("content/official/sfx/cast_successful.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::CAST_UNSUCCESSFUL];
		buf.from_file("content/official/sfx/cast_unsuccessful.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::EXPLOSION];
		buf.from_file("content/official/sfx/explosion.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::GREAT_EXPLOSION];
		buf.from_file("content/official/sfx/great_explosion.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::PED_EXPLOSION];
		buf.from_file("content/official/sfx/ped_explosion.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::INTERFERENCE_EXPLOSION];
		buf.from_file("content/official/sfx/interference_explosion.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::CAST_CHARGING];
		buf.from_file("content/official/sfx/cast_charging.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::GRENADE_UNPIN];
		buf.from_file("content/official/sfx/grenade_unpin.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::GRENADE_THROW];
		buf.from_file("content/official/sfx/grenade_throw.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::ITEM_THROW];
		buf.from_file("content/official/sfx/item_throw_%x.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::COLLISION_METAL_METAL];
		buf.from_file("content/official/sfx/collision_metal_metal_%x.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::COLLISION_METAL_WOOD];
		buf.from_file("content/official/sfx/collision_metal_wood_%x.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::COLLISION_GRENADE];
		buf.from_file("content/official/sfx/collision_grenade.wav");
	}
}