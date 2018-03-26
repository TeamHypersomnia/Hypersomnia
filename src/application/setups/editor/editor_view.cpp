#include "application/setups/editor/editor_view.h"

void editor_view::toggle_grid() {
	auto& f = show_grid;
	f = !f;
}

