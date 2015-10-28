#include "pool.h"
#include <iostream>
namespace augs {
	namespace threads {
		void pool::worker_thread() {
			std::function<void()> task;
			while (true) {
				std::unique_lock<std::mutex> lock(queue_mutex);

				while (!should_stop && tasks.empty())
					sleeping_workers.wait(lock);

				if (should_stop) 
					return;

				task = tasks.front();
				tasks.pop_front();
				task();
			}
		}

		pool::~pool() {
			should_stop = true;
			join_all();
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
				if(worker.joinable()) 
					worker.join();
		}

		void pool::enqueue_limit_yield(size_t max_tasks_count, const std::function<void()>& new_task) {
			while (!should_stop && tasks.size() >= max_tasks_count) std::this_thread::yield();
			
			enqueue(new_task);
		}

		void pool::enqueue_limit_busy(size_t max_tasks_count, const std::function<void()>& new_task) {
			while (!should_stop && tasks.size() >= max_tasks_count);
			
			enqueue(new_task);
		}
	}
}

#include <gtest\gtest.h>

TEST(LimitedThreadPool, MultipleTasksSuccess) {
	augs::threads::pool my_pool;
	
	std::vector<int> mytab;
	mytab.resize(250 * 100000);
	
	auto fill = [&mytab](int i, int j) {
		while (i < j) {
			mytab[i] = 2;
			++i;
		}
	};
	
	for (int i = 0; i < 250; ++i) {
		my_pool.enqueue_limit_yield(2, std::bind(fill, i * 100000, (i + 1) * 100000));
		EXPECT_LE(my_pool.tasks.size(), 2u);
	}
	
	my_pool.enqueue_exit_message();
	my_pool.join_all();
	my_pool.join_all();
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