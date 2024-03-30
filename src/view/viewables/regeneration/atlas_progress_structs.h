#pragma once
#include <atomic>

struct atlas_progress_structs {
	std::atomic<int> current_neon_map_num;
	std::atomic<int> max_neon_maps;
};

struct sound_progress_structs {
	std::atomic<int> sound_req_num;
	std::atomic<int> current_sound_num;
	std::atomic<int> max_sounds;
};
