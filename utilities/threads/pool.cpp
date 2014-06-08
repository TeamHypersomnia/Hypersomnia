#include "pool.h"
#include <iostream>
namespace augs {
	namespace threads {
		bool pool::acquire_new_task(std::function<void()>& task) {
			std::unique_lock<std::mutex> lock(queue_mutex);

			while (!should_stop && tasks.empty())
				sleeping_workers.wait(lock);

			if (should_stop) {
				return false;
			}

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

		void pool::enqueue_exit_message() {
			enqueue([this](){ should_stop = true; sleeping_workers.notify_all(); });
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

		void pool::join_all() {
			for (auto& worker : workers)
				worker.join();
		}

		limited_pool::limited_pool(size_t max_tasks_count, size_t num_of_workers) : max_tasks_count(max_tasks_count), pool(num_of_workers) {}

		void limited_pool::enqueue_with_limit(const std::function<void()>& new_task) {
			while (!should_stop && tasks.size() >= max_tasks_count)
				std::this_thread::yield();
			
			{
				std::unique_lock<std::mutex> lock(queue_mutex);
				tasks.push_back(new_task);
			}

			sleeping_workers.notify_one();
		}

	}
}

#include <gtest\gtest.h>

TEST(LimitedThreadPool, MultipleTasksSuccess) {
	augs::threads::limited_pool my_pool(2);
	
	std::vector<int> mytab;
	mytab.resize(250 * 100000);
	
	auto fill = [&mytab](int i, int j) {
		while (i < j) {
			mytab[i] = 2;
			++i;
		}
	};
	
	for (int i = 0; i < 250; ++i) {
		my_pool.enqueue_with_limit(std::bind(fill, i * 100000, (i + 1) * 100000));
		EXPECT_LE(my_pool.tasks.size(), my_pool.max_tasks_count);
	}
	
	my_pool.enqueue_exit_message();
	my_pool.join_all();
	
	bool all_equals_two = true;
	
	for (auto& val : mytab) {
		if (val != 2) {
			all_equals_two = false;
			break;
		}
	}
	
	EXPECT_EQ(true, all_equals_two);
}