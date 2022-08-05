#pragma once

void create_lights(editor_resource_pools& pools) {
	{
		auto& strong_lamp = create_official(official_lights::STRONG_LAMP, pools);
		(void)strong_lamp;
	}

	{
		auto& aquarium_lamp = create_official(official_lights::AQUARIUM_LAMP, pools).editable;
		aquarium_lamp.attenuation.constant = 75;
		aquarium_lamp.attenuation.quadratic = 631;
	}
}

