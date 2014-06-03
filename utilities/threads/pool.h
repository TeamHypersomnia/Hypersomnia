#include <functional>
#include <thread>
#include <mutex>
#include <vector>

#include <deque>
#include <condition_variable>

namespace augs {
	namespace threads {
		class pool {
			void worker_thread();
		public:
			bool should_stop;

			std::vector<std::thread> workers;
			std::deque<std::function<void()>> tasks;

			void enqueue(const std::function<void()>&);

			pool(size_t num_of_workers = std::thread::hardware_concurrency());

			std::mutex queue_mutex;
			std::condition_variable condition;
		};
	}
}