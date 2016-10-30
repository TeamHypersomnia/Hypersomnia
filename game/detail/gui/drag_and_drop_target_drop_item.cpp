#include "drag_and_drop_target_drop_item.h"

drag_and_drop_target_drop_item::drag_and_drop_target_drop_item(const augs::gui::material new_mat) 
	: mat(new_mat) {
	unset_flag(augs::gui::flag::CLIP); 
	unset_flag(augs::gui::flag::ENABLE_DRAWING);
}