#pragma once

namespace augs {
	class world;
}

/* 
A scene builder is that which was inconvenient to put into system,
so a dummy testbed script for example, some traffic meters, whatever thing that does not deserve its component+system pair.

A scene builder is that which cannot or need not be composed.

Avoid usage of perform_logic_step + draw whenever possible.

*/
#include "../messages/camera_render_request_message.h"

struct scene_builder {
	virtual void initialize(augs::world& world);
	virtual void perform_logic_step(augs::world& world);
	virtual void custom_drawcalls(augs::world& world);

	virtual void execute_drawcall_script(messages::camera_render_request_message);
};
