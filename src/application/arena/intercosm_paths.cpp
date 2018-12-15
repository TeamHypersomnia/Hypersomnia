#include "application/arena/intercosm_paths.h"

intercosm_paths::intercosm_paths(
	const augs::path_type& target_folder,
	const std::string& intercosm_name
) {
	auto in_folder = [&](const auto ext) {
		return target_folder / (intercosm_name + ext);
	};

	viewables_file = in_folder(".viewables");
	solv_file = in_folder(".solv");
	comm_file = in_folder(".comm");
}
