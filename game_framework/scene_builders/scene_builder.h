#pragma once

namespace augs {
	class world;
}

/* 
A scene builder is that which was inconvenient to put into system,
so a dummy testbed setup for example.

In these classes you will find some traffic meters, whatever thing that does not deserve its component+system pair.

A scene builder cannot or need not be composed with other scene builders.

*/
#include "../messages/camera_render_request_message.h"

struct scene_builder {
	virtual void initialize(augs::world& world);
	virtual void perform_logic_step(augs::world& world);
	virtual void custom_drawcalls(augs::world& world);

	virtual void execute_drawcall_script(messages::camera_render_request_message);
};
