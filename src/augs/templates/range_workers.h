#pragma once
#include <vector>
#include <thread>

#include "3rdparty/concurrentqueue/concurrentqueue.h"
#include "augs/templates/traits/function_traits.h"

namespace augs {
	template <class Callback>
	struct range_workers {
		using element_type = std::remove_reference_t<argument_t<Callback, 0>>;
		using Item = element_type*;

		moodycamel::ConcurrentQueue<Item> q;

		std::optional<Callback> callback;

		std::vector<std::thread> workers;
		std::atomic<int> doneConsumers = 0;

		auto make_worker_function() {
			return [&]() {
				Item item;

				do {
					while (q.try_dequeue(item)) {
						(*callback)(*item);
					}

					// Loop again one last time if we're the last producer (with the acquired
					// memory effects of the other producers):
				} while (doneConsumers.fetch_add(1, std::memory_order_acq_rel) + 1 == static_cast<int>(workers.size()));
			};
		}

		range_workers() {
		
		}

		template <class C, class R>
		void process(C&& call, R& range) {
			callback.emplace(std::forward<C>(call));
			doneConsumers = 0;

			for (auto& r : range) {
				q.enqueue(std::addressof(r));
			}

			const auto n = std::thread::hardware_concurrency() * 2;
			workers.reserve(n);

			for (unsigned i = 0; i < n; ++i) {
				workers.emplace_back(make_worker_function());
			}

			make_worker_function()();
			
			for (auto& t : workers) {
				t.join();
			}

			callback.reset();
		}
	};
}
