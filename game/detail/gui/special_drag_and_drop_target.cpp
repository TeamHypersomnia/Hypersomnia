#include "special_drag_and_drop_target.h"

special_drag_and_drop_target::special_drag_and_drop_target(augs::gui::gui_element_id this_id, augs::gui::material new_mat) 
	: rect_leaf(this_id), mat(new_mat), type(special_control::DROP_ITEM) {
	unset_flag(flag::CLIP); 
	unset_flag(flag::ENABLE_DRAWING);
}