#include "application/arena/arena_paths.h"

arena_paths::arena_paths(
	const augs::path_type& target_folder,
	const std::string& arena_name
) {
	auto in_folder = [&](const auto ext) {
		return target_folder / (arena_name + ext);
	};

	int_file = in_folder(".int");
	rulesets_file = in_folder(".rulesets");
}

arena_paths::arena_paths(const std::string& arena_name) 
	: arena_paths(augs::path_type(ARENAS_DIR) / arena_name, arena_name) 
{

}
