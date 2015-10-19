#include "stdafx.h"
#include "file_watcher.h"

namespace augs {
	namespace misc {
		misc::vector_wrapper<std::string> file_watcher::get_modified_files() {
			fw.update();

			std::unique_lock<std::mutex> lock(lock);
			misc::vector_wrapper<std::string> output;
			output.raw = filenames;
			filenames.clear();

			return output;
		}

		void file_watcher::add_directory(const std::string& wdir) {
			fw.addWatch(wdir, &listener);
		}
	}
}