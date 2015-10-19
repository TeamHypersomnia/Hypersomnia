#pragma once
#define UNICODE
#include "vector_wrapper.h"

#include "FileWatcher/FileWatcher.h"
#include "timer.h"
#include <functional>
#include <mutex>

namespace augs {
	namespace misc {
		class file_watcher {
			struct update_listener : public FW::FileWatchListener {
				std::function<void(std::string)> callback;
				void handleFileAction(FW::WatchID watchid, const FW::String& dir, const FW::String& filename, FW::Action action) { callback( filename ); }
			};

			std::vector<std::string> filenames;
			std::mutex lock;

			timer notification_interval;
			FW::FileWatcher fw;
			
			update_listener listener;


		public:
			file_watcher() {
				listener.callback = [this](std::string filename) {
					std::unique_lock<std::mutex> lock(lock);
					filenames.push_back(filename);
				};
			}

			misc::vector_wrapper<std::string> get_modified_files();
			void add_directory(const std::string& directory);
		};
	}
}
