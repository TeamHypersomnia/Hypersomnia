#include "pool.h"

namespace augs {
	namespace threads {
		bool pool::acquire_new_task(std::function<void()>& task) {
			std::unique_lock<std::mutex> lock(queue_mutex);

			while (!should_stop && tasks.empty())
				sleeping_workers.wait(lock);

			if (should_stop)
				return false;

			task = tasks.front();
			tasks.pop_front();

			return true;
		}

		void pool::worker_thread() {
			std::function<void()> task;
			while(true) {
				if(!acquire_new_task(task)) return;

				task();
			}
		}

		void pool::enqueue(const std::function<void()>& new_task) {
			{
				std::unique_lock<std::mutex> lock(queue_mutex);

				tasks.push_back(new_task);
			}

			sleeping_workers.notify_one();
		}


		pool::pool(size_t num_of_workers) : should_stop(false) {
			for (size_t i = 0; i < num_of_workers; ++i) 
				workers.push_back(std::thread(std::bind(&pool::worker_thread, this)));
		}

		void limited_pool::worker_thread() {
			std::function<void()> task;

			while (true) {
				if (!acquire_new_task(task)) return;
				task();
				sleeping_enqueuers.notify_one();
			}
		}

		void limited_pool::enqueue_with_limit(const std::function<void()>& new_task) {
			{
				std::unique_lock<std::mutex> lock(queue_mutex);

				while (!should_stop && tasks.size() >= max_tasks_count)
					sleeping_enqueuers.wait(lock);

				tasks.push_back(new_task);
			}

			sleeping_workers.notify_one();
		}
	}
}