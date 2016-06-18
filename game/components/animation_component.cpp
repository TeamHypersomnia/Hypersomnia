#include "animation_component.h"

namespace components {
	unsigned animation::get_current_frame() const {
		return frame_num;
	}

	void animation::increase_frame(entity_id sub) {
		set_current_frame(frame_num + 1);
	}

	void animation::decrease_frame(entity_id sub) {
		set_current_frame(frame_num - 1);
	}
}