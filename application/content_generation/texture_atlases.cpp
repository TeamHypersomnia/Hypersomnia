#include "texture_atlases.h"

#include <sstream>
#include <experimental/filesystem>

#include "game/resources/manager.h"
#include "augs/filesystem/directory.h"
#include "augs/filesystem/file.h"

#include "augs/ensure.h"
#include "augs/misc/streams.h"

#include "augs/image/image.h"

atlases_regeneration_output regenerate_atlases(const atlases_regeneration_input& in) {
	atlases_regeneration_output output;

	auto& manager = get_resource_manager();

	const auto atlases_directory = std::string("generated/atlases/");
	const auto atlases_stamp_filename = atlases_directory + "atlases.stamp";
	const auto atlases_metadata_filename = atlases_directory + "atlases.meta";

	bool should_regenerate_all = false;

	if (!augs::file_exists(atlases_stamp_filename)) {
		should_regenerate_all = true;
	}
	else {
		augs::stream stamp_stream;

		augs::assign_file_contents_binary(atlases_stamp_filename, stamp_stream);
	}

	if (should_regenerate_all) {

	}
}