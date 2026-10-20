#pragma once
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <chrono>
#include "augs/image/image.h"
#include "augs/texture_atlas/atlas_entry.h"

/*
	Small miniatures of arenas shown left of the arena names in the arena choosers -
	all in one texture (imgui_atlas_type::ARENA_MINIATURES), a grid of cells, one arena each.

	A chooser asks for its arena by name every frame it is drawn.
	The game uploads the grid again (take_new_grid) whenever an arena not seen yet was asked for -
	its miniature.png is looked up like a server looks for an arena: official, user projects, downloaded.
	A full grid is started anew with only the newly asked arenas - the others ask again on the next frame.
	Not more often than every few seconds, so that more arenas than cells on one screen don't rebuild it every frame:
	until then, the ones that don't fit show no miniature.
*/

class arena_miniatures {
	static constexpr unsigned cell_size_v = 96;
	static constexpr unsigned cells_per_row_v = 16;
	static constexpr unsigned max_cells_v = cells_per_row_v * cells_per_row_v;

	augs::image grid;

	/*
		std::nullopt for an arena without a miniature on disk, so that it isn't looked up again every frame.
	*/

	std::unordered_map<std::string, std::optional<augs::atlas_entry>> known;
	std::vector<std::string> pending;
	unsigned used_cells = 0;
	std::optional<std::chrono::steady_clock::time_point> when_started_anew;

	void place(const std::string& arena_name);

public:
	const augs::atlas_entry* request(const std::string& arena_name);
	std::optional<augs::image> take_new_grid();
};

arena_miniatures& get_arena_miniatures();
