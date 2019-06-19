#pragma once
#include <atomic>

struct atlas_progress_structs {
	std::atomic<int> current_neon_map_num;
	std::atomic<int> max_neon_maps;
};
