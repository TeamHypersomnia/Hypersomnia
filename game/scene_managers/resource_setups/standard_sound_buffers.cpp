#include "all.h"
#include "game/resources/manager.h"
#include "game/enums/sound_response_type.h"

#include <sndfile.h>

namespace resource_setups {
	void load_standard_sound_buffers() {
		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::BILMER2000_MUZZLE);
			buf.from_file("hypersomnia/sfx/bilmer2000_muzzle.wav");
		}

		{
			auto& buf = get_resource_manager().create(assets::sound_buffer_id::ELECTRIC_PROJECTILE_FLIGHT);
			buf.from_file("hypersomnia/sfx/electric_projectile_flight.wav");
		}

		{
			auto& res = get_resource_manager().create(assets::sound_response_id::BILMER2000_RESPONSE);
			res[sound_response_type::MUZZLE_SHOT] = assets::sound_buffer_id::BILMER2000_MUZZLE;
		}

		{
			auto& res = get_resource_manager().create(assets::sound_response_id::KEK9_RESPONSE);
			res[sound_response_type::MUZZLE_SHOT] = assets::sound_buffer_id::BILMER2000_MUZZLE;
		}

		{
			auto& res = get_resource_manager().create(assets::sound_response_id::ASSAULT_RIFLE_RESPONSE);
			res[sound_response_type::MUZZLE_SHOT] = assets::sound_buffer_id::BILMER2000_MUZZLE;
		}

		{
			auto& res = get_resource_manager().create(assets::sound_response_id::SUBMACHINE_RESPONSE);
			res[sound_response_type::MUZZLE_SHOT] = assets::sound_buffer_id::BILMER2000_MUZZLE;
		}

		{
			auto& res = get_resource_manager().create(assets::sound_response_id::ELECTRIC_PROJECTILE_RESPONSE);
			res[sound_response_type::PROJECTILE_TRACE] = assets::sound_buffer_id::ELECTRIC_PROJECTILE_FLIGHT;
		}
	}
}