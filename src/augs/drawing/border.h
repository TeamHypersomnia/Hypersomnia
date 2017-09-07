#pragma once

struct border_input {
	int width = 1;
	int spacing = 1;

	auto get_total_expansion() const {
		return width + spacing;
	}
};
