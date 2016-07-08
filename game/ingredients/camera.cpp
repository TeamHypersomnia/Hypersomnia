#include "ingredients.h"
#include "game/entity_handle.h"
#include "game/cosmos.h"

#include "game/components/position_copying_component.h"
#include "game/components/camera_component.h"
#include "game/components/input_receiver_component.h"

namespace ingredients {
	void camera(entity_handle e, int w, int h) {
		components::transform transform;
		components::input_receiver input;
		components::camera camera;
		components::position_copying position_copying;

		camera.enable_smoothing = true;
		camera.smoothing_average_factor = 0.5;
		camera.averages_per_sec = 25;

		camera.orbit_mode = camera.LOOK;
		camera.max_look_expand.set(w / 2, h / 2);
		camera.angled_look_length = 10;

		camera.visible_world_area.set(w, h);

		position_copying.relative = false;

		e.add(transform);
		e.add(input);
		e.add(camera);
		e.add(position_copying);
	}
}