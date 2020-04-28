#include "application/arena/arena_paths.h"

arena_paths::arena_paths(
	const augs::path_type& target_folder,
	const std::string& arena_name
) : int_paths(target_folder, arena_name), folder_path(target_folder) {
	auto in_folder = [&](const auto ext) {
		return target_folder / (arena_name + ext);
	};

	rulesets_file_path = in_folder(".rulesets");
	miniature_file_path = in_folder(".miniature.png");
}

arena_paths::arena_paths(const std::string& arena_name) 
	: arena_paths(augs::path_type(OFFICIAL_ARENAS_DIR) / arena_name, arena_name) 
{

}
