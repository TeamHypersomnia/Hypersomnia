#pragma once

void create_sounds(editor_resource_pools& pools) {
	auto create_sound = [&](const official_sounds id) -> auto& {
		return create_official(id, pools).editable;
	};

	{
		auto& loudy_fan = create_sound(official_sounds::LOUDY_FAN);

		loudy_fan.distance_model = augs::distance_model::LINEAR_DISTANCE_CLAMPED;
		loudy_fan.reference_distance = 20.0f;
		loudy_fan.max_distance = 400.0f;
		loudy_fan.doppler_factor = 1.0f;
		loudy_fan.gain = 0.5f;
	}

	create_sound(official_sounds::AQUARIUM_AMBIENCE_LEFT);
	create_sound(official_sounds::AQUARIUM_AMBIENCE_RIGHT);

	create_sound(official_sounds::GLASS_DAMAGE);
	create_sound(official_sounds::COLLISION_GLASS);
}

