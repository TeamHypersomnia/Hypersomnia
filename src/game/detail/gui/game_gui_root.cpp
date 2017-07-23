#include <tuple>
#include "game_gui_root.h"

game_gui_root::game_gui_root(const vec2i screen_size) {
	unset_flag(augs::gui::flag::CLIP);
	set_flag(augs::gui::flag::ENABLE_DRAWING_OF_CHILDREN);
	set_flag(augs::gui::flag::DISABLE_HOVERING);

	rc = xywh(0, 0, 0, 0);
	set_screen_size(screen_size);
}

void game_gui_root::set_screen_size(const vec2i screen_size) {

}
