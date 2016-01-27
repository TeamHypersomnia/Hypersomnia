#include "file_watcher.h"

namespace augs {
	namespace misc {
		std::vector<std::wstring> file_watcher::get_modified_files() {
			fw.update();

			std::unique_lock<std::mutex> lock(lock);
			auto output = filenames;
			filenames.clear();

			return output;
		}

		void file_watcher::add_directory(const std::wstring& wdir) {
			fw.addWatch(wdir, &listener);
		}
	}
}