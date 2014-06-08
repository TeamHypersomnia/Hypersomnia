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
			bool acquire_new_task(std::function<void()>& out);
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

			void join_all();
		};

		class limited_pool : public pool {
			void worker_thread();
		public:
			size_t max_tasks_count;
			
			limited_pool(size_t max_tasks_count, size_t num_of_workers = std::thread::hardware_concurrency());
			
			void enqueue_with_limit(const std::function<void()>&);
		};
	}
}