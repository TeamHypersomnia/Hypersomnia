#include "pool.h"

namespace augs {
	namespace threads {
		void pool::worker_thread() {
			std::function<void()> task;
			while(true) {
				{   
					std::unique_lock<std::mutex> lock(queue_mutex);

					while (!should_stop && tasks.empty())
						condition.wait(lock);

					if (should_stop)
						return;

					task = tasks.front();
					tasks.pop_front();
				}

				task();
			}
		}

		void pool::enqueue(const std::function<void()>& new_task) {
			{
				std::unique_lock<std::mutex> lock(queue_mutex);

				tasks.push_back(new_task);
			}

			condition.notify_one();
		}

		pool::pool(size_t num_of_workers) : should_stop(false) {
			for (size_t i = 0; i < num_of_workers; ++i) 
				workers.push_back(std::thread(std::bind(&pool::worker_thread, this)));
		}
	}
}