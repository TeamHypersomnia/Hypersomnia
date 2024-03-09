#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>

namespace augs {
	class thread_pool {
		std::vector<std::thread> workers;
		std::vector<std::function<void()>> tasks;
		std::vector<std::function<void()>> cold_tasks;

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

		auto lock_completion() {
			return std::unique_lock<std::mutex>(completion_mutex);
		}

		void register_completion() {
			{
				auto lock = lock_completion();
				++tasks_completed;

				if (tasks_completed == tasks_posted) {
					completion_variable.notify_all();
				}
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

						task = std::move(tasks.back());
						tasks.pop_back();
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
		void enqueue(F&& f) {
			cold_tasks.emplace_back(std::move(f));
		}

		void submit() {
			{
				auto lock = lock_queue();
				ensure(tasks.empty());
				std::swap(cold_tasks, tasks);

				{
					auto lock = lock_completion();
					tasks_completed = 0;
					tasks_posted = tasks.size();
				}
			}

			cold_tasks.clear();
			completion_variable.notify_all();
			cv.notify_all();
		}

		std::size_t size() const {
			return workers.size();
		}

		void help_until_no_tasks() {
			for (;;) {
				std::function<void()> task;

				{
					auto lock = lock_queue();

					if (tasks.empty()) {
						return;
					}

					task = std::move(tasks.back());
					tasks.pop_back();
				}

				task();
				register_completion();
			}
		}

		void wait_for_all_tasks_to_complete() {
			auto lock = lock_completion();
			completion_variable.wait(lock, [this]{ return tasks_posted == tasks_completed; });
		}
	};
}

