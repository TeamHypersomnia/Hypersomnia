#include <functional>
#include <thread>
#include <mutex>
#include <vector>

#include <deque>
#include <condition_variable>

namespace augs {
	namespace threads {
		class pool {
		protected:
			void worker_thread();
		public:
			bool should_stop;

			std::vector<std::thread> workers;
			std::deque<std::function<void()>> tasks;

			void enqueue(const std::function<void()>&);
			void enqueue_exit_message();

			pool(size_t num_of_workers = std::thread::hardware_concurrency());

			std::mutex queue_mutex;
			std::condition_variable sleeping_workers;

			void enqueue_limit_yield(size_t max_tasks_count, const std::function<void()>&);
			void enqueue_limit_busy (size_t max_tasks_count, const std::function<void()>&);

			void join_all();
		};
	}
}