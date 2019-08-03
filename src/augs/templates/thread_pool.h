#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>

namespace augs {
	class thread_pool {
		std::vector<std::thread> workers;
		std::queue<std::function<void()>> tasks;

		int tasks_completed = 0;
		int tasks_posted = 0;

		std::mutex queue_mutex;
		std::condition_variable cv;

		std::condition_variable completion_variable;
		std::mutex completion_mutex;

		std::atomic<bool> shall_quit = false;

		auto lock_queue() {
			return std::unique_lock<std::mutex>(queue_mutex);
		}

		void register_completion() {
			{
				auto lock = std::unique_lock<std::mutex>(completion_mutex);
				++tasks_completed;
			}

			if (all_tasks_completed()) {
				completion_variable.notify_all();
			}
		}

		auto make_continuous_worker() {
			return [this] {
				for (;;) {
					std::function<void()> task;

					{
						auto lock = lock_queue();
						cv.wait(lock, [this]{ return shall_quit || !tasks.empty(); });

						if (shall_quit.load() && tasks.empty()) {
							return;
						}

						task = std::move(tasks.front());
						tasks.pop();
					}

					task();
					register_completion();
				}
			};
		}

		void join_all() {
			for (auto& worker : workers) {
				worker.join();
			}
		}

		void quit_all_workers() {
			if (workers.empty()) {
				return;
			}

			shall_quit.store(true);
			cv.notify_all();
			join_all();
			workers.clear();
		}

	public:
		thread_pool(const std::size_t num_workers) {
			resize(num_workers);
		}

		~thread_pool() {
			quit_all_workers();
		}

		void resize(const std::size_t num_workers) {
			quit_all_workers();
			shall_quit.store(false);

			for (std::size_t i = 0; i < num_workers; ++i) {
				workers.emplace_back(make_continuous_worker());
			}
		}

		template <class F>
		void enqueue_multiple(F job_provider) {
			int now_posted = 0;

			auto enqueuer = [&](auto&& new_job) {
				tasks.emplace(std::forward<decltype(new_job)>(new_job));
				++now_posted;
				++tasks_posted;
			};

			{
				auto lock = lock_queue();
				job_provider(enqueuer);
			}

			if (now_posted >= static_cast<int>(size())) {
				cv.notify_all();
			}
			else {
				while (now_posted--) {
					cv.notify_one();
				}
			}
		}

		template <class F>
		void enqueue(F&& f) {
			{
				auto lock = lock_queue();
				tasks.emplace(std::forward<F>(f));
				++tasks_posted;
			}

			cv.notify_one();
		}

		void help_until_no_tasks() {
			for (;;) {
				std::function<void()> task;

				{
					auto lock = lock_queue();

					if (tasks.empty()) {
						return;
					}

					task = std::move(tasks.front());
					tasks.pop();
				}

				task();
				register_completion();
			}
		}

		std::size_t size() const {
			return workers.size();
		}

		bool all_tasks_completed() const {
			return tasks_completed == tasks_posted;
		}

		void wait_for_all_tasks_to_complete() {
			auto lock = std::unique_lock<std::mutex>(completion_mutex);
			completion_variable.wait(lock, [this]{ return all_tasks_completed(); });

			tasks_completed = 0;
			tasks_posted = 0;
		}
	};
}

