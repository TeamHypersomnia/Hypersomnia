#pragma once
#define UNICODE
#include <vector>
#include "readdir/ReadDirectoryChanges.h"
#include "vector_wrapper.h"

#include "timer.h"

namespace augs {
	namespace misc {
		class file_watcher {
			timer notification_interval;
			CReadDirectoryChanges changes;
		public:
			misc::vector_wrapper<std::string> get_modified_files();
			void add_directory(const std::wstring& directory, bool subtree);
		};
	}
}
