#pragma once
#include <vector>
#include "FileWatcher/FileWatcher.h"
#include "timer.h"
#include <functional>
#include <mutex>

namespace augs {
	namespace misc {
		class file_watcher {
			struct update_listener : public FW::FileWatchListener {
				std::function<void(std::wstring)> callback;
				void handleFileAction(FW::WatchID watchid, const FW::String& dir, const FW::String& filename, FW::Action action) { callback( filename ); }
			};

			std::vector<std::wstring> filenames;
			std::mutex lock;

			timer notification_interval;
			FW::FileWatcher fw;
			
			update_listener listener;


		public:
			file_watcher() {
				listener.callback = [this](std::wstring filename) {
					std::unique_lock<std::mutex> lock(lock);
					filenames.push_back(filename);
				};
			}

			std::vector<std::wstring> get_modified_files();
			void add_directory(const std::wstring& directory);
		};
	}
}
