#include "all.h"
#include "game/resources/manager.h"
#include "game/enums/sound_response_type.h"
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
			buf.from_file("hypersomnia/sfx/bilmer2000_muzzle.wav");
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

		{
			auto& res = get_resource_manager().create(assets::sound_response_id::BILMER2000_RESPONSE);
			augs::sound_effect_modifier mod;
			mod.max_distance = 1920.f * 3.f;
			mod.reference_distance = 0.f;
			mod.gain = 1.3f;
			res[sound_response_type::MUZZLE_SHOT] = {assets::sound_buffer_id::BILMER2000_MUZZLE, mod};
		}

		{
			auto& res = get_resource_manager().create(assets::sound_response_id::KEK9_RESPONSE);
			res[sound_response_type::MUZZLE_SHOT] = assets::sound_buffer_id::KEK9_MUZZLE;
		}

		{
			auto& res = get_resource_manager().create(assets::sound_response_id::ASSAULT_RIFLE_RESPONSE);
			res[sound_response_type::MUZZLE_SHOT] = assets::sound_buffer_id::ASSAULT_RIFLE_MUZZLE;
		}

		{
			auto& res = get_resource_manager().create(assets::sound_response_id::SUBMACHINE_RESPONSE);
			res[sound_response_type::MUZZLE_SHOT] = assets::sound_buffer_id::BILMER2000_MUZZLE;
		}

		{
			auto& res = get_resource_manager().create(assets::sound_response_id::ELECTRIC_PROJECTILE_RESPONSE);
			
			augs::sound_effect_modifier trace_modifier;
			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.gain = 1.3f;
			trace_modifier.repetitions = -1;
			trace_modifier.fade_on_exit = false;
			res[sound_response_type::PROJECTILE_TRACE] = { assets::sound_buffer_id::ELECTRIC_PROJECTILE_FLIGHT, trace_modifier };
			res[sound_response_type::DESTRUCTION_EXPLOSION] = assets::sound_buffer_id::ELECTRIC_DISCHARGE_EXPLOSION;
		}

		{
			auto& res = get_resource_manager().create(assets::sound_response_id::CHARACTER_RESPONSE);
			res[sound_response_type::HEALTH_DECREASE] = assets::sound_buffer_id::IMPACT;
			res[sound_response_type::DEATH] = assets::sound_buffer_id::DEATH;
		}
	}
}