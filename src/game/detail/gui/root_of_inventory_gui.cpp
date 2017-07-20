#include <tuple>
#include "root_of_inventory_gui.h"

root_of_inventory_gui::root_of_inventory_gui(const vec2i screen_size) {
	unset_flag(augs::gui::flag::CLIP);
	set_flag(augs::gui::flag::ENABLE_DRAWING_OF_CHILDREN);
	set_flag(augs::gui::flag::DISABLE_HOVERING);

	rc = xywh(0, 0, 0, 0);
	set_screen_size(screen_size);
}

void root_of_inventory_gui::set_screen_size(const vec2i screen_size) {

}
