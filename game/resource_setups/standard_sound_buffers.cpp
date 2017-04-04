#include "all.h"
#include "game/resources/manager.h"
#include "augs/audio/sound_effect_modifier.h"

#include <sndfile.h>

namespace resource_setups {
	void load_standard_sound_buffers() {
		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::BILMER2000_MUZZLE);
			buf.from_file("hypersomnia/sfx/bilmer2000_muzzle.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::ASSAULT_RIFLE_MUZZLE);
			buf.from_file("hypersomnia/sfx/assault_muzzle.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::SUBMACHINE_MUZZLE);
			buf.from_file("hypersomnia/sfx/submachine_muzzle.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::KEK9_MUZZLE);
			buf.from_file("hypersomnia/sfx/kek9_muzzle.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::ELECTRIC_PROJECTILE_FLIGHT);
			buf.from_file("hypersomnia/sfx/electric_projectile_flight.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::BULLET_PASSES_THROUGH_HELD_ITEM);
			buf.from_file("hypersomnia/sfx/bullet_hits_held_item.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::IMPACT);
			buf.from_file("hypersomnia/sfx/impact_light_%x.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::DEATH);
			buf.from_file("hypersomnia/sfx/impact_%x.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::ENGINE);
			buf.from_file("hypersomnia/sfx/engine.ogg");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::ELECTRIC_DISCHARGE_EXPLOSION);
			buf.from_file("hypersomnia/sfx/electric_discharge_explosion_%x.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::WIND);
			buf.from_file("hypersomnia/sfx/wind_%x.ogg");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::BUTTON_HOVER);
			buf.from_file("hypersomnia/sfx/button_hover.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::BUTTON_CLICK);
			buf.from_file("hypersomnia/sfx/button_click.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::LOW_AMMO_CUE);
			buf.from_file("hypersomnia/sfx/low_ammo_cue.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::FIREARM_ENGINE);
			buf.from_file("hypersomnia/sfx/firearm_engine.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::CAST_SUCCESSFUL);
			buf.from_file("hypersomnia/sfx/cast_successful.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::CAST_UNSUCCESSFUL);
			buf.from_file("hypersomnia/sfx/cast_unsuccessful.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::EXPLOSION);
			buf.from_file("hypersomnia/sfx/explosion.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::GREAT_EXPLOSION);
			buf.from_file("hypersomnia/sfx/great_explosion.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::PED_EXPLOSION);
			buf.from_file("hypersomnia/sfx/ped_explosion.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::INTERFERENCE_EXPLOSION);
			buf.from_file("hypersomnia/sfx/interference_explosion.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::CAST_CHARGING);
			buf.from_file("hypersomnia/sfx/cast_charging.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::GRENADE_UNPIN);
			buf.from_file("hypersomnia/sfx/grenade_unpin.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::GRENADE_THROW);
			buf.from_file("hypersomnia/sfx/grenade_throw.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::ITEM_THROW);
			buf.from_file("hypersomnia/sfx/item_throw.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::COLLISION_METAL_METAL);
			buf.from_file("hypersomnia/sfx/collision_metal_metal_%x.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::COLLISION_METAL_WOOD);
			buf.from_file("hypersomnia/sfx/collision_metal_wood_%x.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::COLLISION_GRENADE);
			buf.from_file("hypersomnia/sfx/collision_grenade.wav");
		}
	}
}