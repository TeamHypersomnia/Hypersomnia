#include "view/game_gui/elements/gui_grid.h"

#define GRID_WIDTH 11

vec2i griddify(const vec2 v) {
	return griddify(vec2i(v));
}

vec2i griddify(vec2i vi) {
	int prev = vi.x / GRID_WIDTH;
	int next = prev + 1;
	prev *= GRID_WIDTH;
	next *= GRID_WIDTH;

	if (vi.x - prev < next - vi.x) {
		vi.x = prev;
	}
	else vi.x = next;

	prev = vi.y / GRID_WIDTH;
	next = prev + 1;
	prev *= GRID_WIDTH;
	next *= GRID_WIDTH;

	if (vi.y - prev < next - vi.y) {
		vi.y = prev;
	}
	else vi.y = next;

	return vi;
}