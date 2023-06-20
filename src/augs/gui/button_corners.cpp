#include "augs/templates/enum_introspect.h"
#include "augs/string/string_templates.h"

#include "augs/gui/button_corners.h"

augs::path_type get_filename_for(const button_corner_type t) {
	return to_lowercase(augs::enum_to_string(t));
}

bool is_button_corner(const button_corner_type t) {
	if (
		t == button_corner_type::LT
		|| t == button_corner_type::RT
		|| t == button_corner_type::RB
		|| t == button_corner_type::LB

		|| t == button_corner_type::LT_BORDER
		|| t == button_corner_type::RT_BORDER
		|| t == button_corner_type::RB_BORDER
		|| t == button_corner_type::LB_BORDER

		|| t == button_corner_type::LT_INTERNAL_BORDER
		|| t == button_corner_type::RT_INTERNAL_BORDER
		|| t == button_corner_type::RB_INTERNAL_BORDER
		|| t == button_corner_type::LB_INTERNAL_BORDER
	) {
		return true;
	}

	return false;
}

bool is_button_outside_border(const button_corner_type t) {
	if (
		t == button_corner_type::L_BORDER
		|| t == button_corner_type::T_BORDER
		|| t == button_corner_type::R_BORDER
		|| t == button_corner_type::B_BORDER

		|| t == button_corner_type::LT_BORDER
		|| t == button_corner_type::RT_BORDER
		|| t == button_corner_type::RB_BORDER
		|| t == button_corner_type::LB_BORDER

		|| t == button_corner_type::LB_COMPLEMENT_BORDER
	) {
		return true;
	}

	return false;
}

bool is_lb_complement(const button_corner_type t) {
	if (
		t == button_corner_type::LB_COMPLEMENT
		|| t == button_corner_type::LB_COMPLEMENT_BORDER
	) {
		return true;
	}

	return false;
}

bool is_button_border(const button_corner_type t) {
	if (
		is_button_outside_border(t)
		|| t == button_corner_type::LT_INTERNAL_BORDER
		|| t == button_corner_type::RT_INTERNAL_BORDER
		|| t == button_corner_type::RB_INTERNAL_BORDER
		|| t == button_corner_type::LB_INTERNAL_BORDER

		) {
		return true;
	}

	return false;
}

bool is_button_side(const button_corner_type t) {
	if (
		t == button_corner_type::L
		|| t == button_corner_type::T
		|| t == button_corner_type::R
		|| t == button_corner_type::B

		|| t == button_corner_type::L_BORDER
		|| t == button_corner_type::T_BORDER
		|| t == button_corner_type::R_BORDER
		|| t == button_corner_type::B_BORDER
	) {
		return true;
	}

	return false;
}