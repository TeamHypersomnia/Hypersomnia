#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "augs/templates/traits/function_traits.h"

namespace augs {
	template <class Callback>
	class range_workers {
		using element_type = std::remove_reference_t<argument_t<Callback, 0>>;
		using Item = element_type*;

		int count = 0;
		element_type* arr = nullptr;

		std::optional<Callback> callback;
		std::vector<std::thread> workers;
		std::atomic<int> it = 0;
		std::atomic<int> num_busy_workers = 0;

		std::mutex m;
		std::condition_variable cv;

		std::atomic<bool> shall_quit = false;

		auto make_worker_function() {
			return [&]() {
				while (true) {
					{
						std::unique_lock<std::mutex> lk(m);
						cv.wait(lk, [this]{ return shall_quit || count; });
					}

					if (shall_quit.load()) {
						return;
					}

					process_tasks();
				}
			};
		}

		void init_workers(const std::size_t n) {
			shall_quit.store(false);
			workers.reserve(n);

			for (std::size_t i = 0; i < n; ++i) {
				workers.emplace_back(make_worker_function());
			}
		}

		void join_all() {
			for (auto& w : workers) {
				w.join();
			}
		}

		void quit_workers() {
			shall_quit.store(true);
			cv.notify_all();
			join_all();
			workers.clear();
		}

		void wait_complete() {
			process_tasks();

			while (num_busy_workers.load()) {
				std::this_thread::yield();
			}
		}

	public:
		range_workers(const std::size_t n) {
			init_workers(n);
		}

		range_workers() : range_workers(std::thread::hardware_concurrency() - 1) {}

		range_workers(const range_workers&) = delete;
		range_workers& operator=(const range_workers&) = delete;

		range_workers(range_workers&&) = delete;
		range_workers& operator=(range_workers&&) = delete;

		auto process_tasks() {
			num_busy_workers.fetch_add(1);

			while (true) {
				const auto i = it.fetch_add(1, std::memory_order_relaxed);

				if (i < count) {
					(*callback)(arr[i]);
				}
				else {
					break;
				}
			}

			num_busy_workers.fetch_sub(1);
		}

		void resize_workers(const std::size_t num) {
			if (num == workers.size()) {
				return;
			}

			quit_workers();
			init_workers(num);
		}

		template <class C, class R>
		void process(C&& call, R& range) {
			{
				std::unique_lock<std::mutex> lk(m);

				it = 0;
				count = range.size();
				arr = range.data();

				callback.emplace(std::forward<C>(call));
			}

			cv.notify_all();
			wait_complete();

			std::unique_lock<std::mutex> lk(m);
			count = 0;
		}

		~range_workers() {
			quit_workers();
		}
	};
}
