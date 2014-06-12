#include "stdafx.h"
#include "file_watcher.h"
#include <algorithm>

namespace augs {
	namespace misc {
		misc::vector_wrapper<std::string> file_watcher::get_modified_files() {
			misc::vector_wrapper<std::string> output;

			if (WaitForSingleObjectEx(changes.GetWaitHandle(), 0, false) == WAIT_OBJECT_0) {
				if (changes.CheckOverflow()) {
					throw std::runtime_error("file change command buffer overflow");
				}
				else {
					DWORD dwAction;
					CStringW wstrFilename;

					std::vector<CStringW> filenames;

					while (changes.Pop(dwAction, wstrFilename))
						if (dwAction == FILE_ACTION_MODIFIED)
							filenames.push_back(wstrFilename);

					if (notification_interval.get<std::chrono::seconds>() > 1.0) {
						/* one file at a time, pls */
						std::sort(filenames.begin(), filenames.end());
						filenames.erase(std::unique(filenames.begin(), filenames.end()), filenames.end());

						for (auto& filename : filenames) {
							auto wwstr = std::wstring(filename);

							output.push_back(std::string(wwstr.begin(), wwstr.end()));
						}
					}

					if (!output.raw.empty())
						notification_interval.reset();
				}
			}

			return output; 
		}

		void file_watcher::add_directory(const std::wstring& wdir, bool subtree) {
			//std::wstring wdir(directory.begin(), directory.end());
			changes.AddDirectory(wdir.c_str(), subtree, FILE_NOTIFY_CHANGE_LAST_WRITE);
		}
	}
}