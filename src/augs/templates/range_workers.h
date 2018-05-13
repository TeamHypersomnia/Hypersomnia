#pragma once
#include <vector>
#include <thread>

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
		std::atomic<int> working_threads = 0;

		std::mutex m;
		std::condition_variable cv;

		std::atomic<bool> shall_quit = false;

	public:
		auto process_tasks() {
			working_threads.fetch_add(1);

			while (true) {
				const auto i = it.fetch_add(1, std::memory_order_relaxed);

				if (i < count) {
					(*callback)(arr[i]);
				}
				else {
					break;
				}
			}

			working_threads.fetch_sub(1);
		}

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

		range_workers() {
			const auto n = std::thread::hardware_concurrency() - 2;

			workers.reserve(n);

			for (unsigned i = 0; i < n; ++i) {
				workers.emplace_back(make_worker_function());
			}
		}

		void join_all() {
			for (auto& w : workers) {
				w.join();
			}
		}

		~range_workers() {
			shall_quit.store(true);
			cv.notify_all();

			join_all();
		}

		void wait_complete() {
			process_tasks();

			while (working_threads.load()) {
				std::this_thread::yield();
			}
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

			count = 0;
		}
	};
}
