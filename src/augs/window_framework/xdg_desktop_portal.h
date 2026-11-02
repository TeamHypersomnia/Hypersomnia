#pragma once
#include <optional>
#include <string>
#include <vector>
#include "augs/filesystem/path_declaration.h"

namespace augs {
	namespace xdg_desktop_portal {
		struct file_filter {
			std::string description;
			std::vector<std::string> patterns;
		};

		enum class file_chooser_mode {
			OPEN_FILE,
			SAVE_FILE,
			CHOOSE_DIRECTORY
		};

		struct file_chooser_request {
			file_chooser_mode mode = file_chooser_mode::OPEN_FILE;
			std::string title;
			std::vector<file_filter> filters;
			std::string suggested_name;
			path_type suggested_folder;
		};

		bool is_service_present();
		bool can_choose(file_chooser_mode);
		std::optional<path_type> choose(const file_chooser_request&);

		bool can_open_directory();
		void open_directory_of(const path_type&);
	}
}
