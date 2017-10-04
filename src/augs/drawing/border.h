#pragma once

struct border_input {
	int width = 1;
	int spacing = 0;

	auto get_total_expansion() const {
		return width + spacing;
	}
};
