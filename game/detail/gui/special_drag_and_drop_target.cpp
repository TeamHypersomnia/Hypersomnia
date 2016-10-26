#include "special_drag_and_drop_target.h"

special_drag_and_drop_target::special_drag_and_drop_target(const augs::gui::material new_mat) 
	: mat(new_mat), type(special_control::DROP_ITEM) {
	unset_flag(augs::gui::flag::CLIP); 
	unset_flag(augs::gui::flag::ENABLE_DRAWING);
}