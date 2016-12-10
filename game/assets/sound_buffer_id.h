#pragma once

namespace augs {
	class sound_buffer;
}

namespace assets {
	enum class sound_buffer_id : unsigned short {
		INVALID,
		BILMER2000_MUZZLE,
		ASSAULT_RIFLE_MUZZLE,
		SUBMACHINE_MUZZLE,
		PISTOL_MUZZLE,
		KEK9_MUZZLE,
		ELECTRIC_PROJECTILE_FLIGHT,
		ELECTRIC_DISCHARGE_EXPLOSION,
		ELECTRIC_ENGINE,
		
		IMPACT,
		DEATH,
		BULLET_PASSES_THROUGH_HELD_ITEM,

		WIND,
		ENGINE,

		COUNT
	};
}

augs::sound_buffer& operator*(const assets::sound_buffer_id& id);
bool operator!(const assets::sound_buffer_id& id);