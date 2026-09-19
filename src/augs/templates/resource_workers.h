#pragma once
#include <cstddef>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <algorithm>
#include <functional>
#include <exception>
#include <condition_variable>

#include "augs/misc/scope_guard.h"

namespace augs {
	enum class resource_priority {
		/* Anything the player is not yet looking at. */
		NORMAL,

		/* Needed before a correct frame can be drawn at all. */
		FIRST_FRAME
	};

	/*
		A parallel-for over an index range, shared by the content loading paths.

		Deliberately not augs::thread_pool. That one is the frame's job list: a single
		producer posts one batch per frame and joins it before the next, which is what
		its ensure(tasks.empty()) asserts. This one has several producers - the atlas
		bake and the sound loading run on separate background threads and overlap - so
		every call carries its own state and waits only for itself.

		Regions run concurrently, and the workers always pick the most urgent one that
		still has work. Nothing here is about fairness: the atlas is what the first
		frame on screen needs, so it takes the whole pool while the sounds - which
		nobody is looking at - progress on their own calling thread meanwhile.

		Strict turn-taking was tried and measured instead. Total loading time came out
		the same, because the sounds dominate it either way, but the atlas went from
		~33 ms to ~200 ms whenever the bake happened to queue up behind them.

		The thread that calls process() takes part, so a pool of zero workers still
		runs everything, just sequentially.
	*/
#if WEB_SINGLETHREAD
	class resource_workers {
	public:
		void start(const std::size_t) {}

		std::size_t size() const {
			return 0;
		}

		template <class F>
		void process(const std::size_t count, F&& callback, const resource_priority = resource_priority::NORMAL) {
			for (std::size_t i = 0; i < count; ++i) {
				callback(i);
			}
		}
	};
#else
	class resource_workers {
		struct region {
			std::function<void(std::size_t)> callback;
			std::size_t count = 0;
			resource_priority priority = resource_priority::NORMAL;

			std::atomic<std::size_t> next = 0;
			std::atomic<int> inside = 0;
			std::atomic<bool> urgency_cleared = false;

			/* The first failure, rethrown by process() once everyone has left. */
			std::exception_ptr error;

			bool all_claimed() const {
				return next.load() >= count;
			}

			bool complete() const {
				return all_claimed() && inside.load() == 0;
			}
		};

		using region_ptr = std::shared_ptr<region>;

		std::vector<std::thread> workers;
		std::vector<region_ptr> regions;

		std::mutex m;
		std::condition_variable work_available;
		std::condition_variable region_finished;

		std::atomic<bool> shall_quit = false;

		/*
			Lets drain() notice an urgent region without taking the lock per item.
		*/
		std::atomic<int> urgent_pending = 0;

		region_ptr find_unclaimed() const {
			region_ptr best;

			for (const auto& r : regions) {
				if (r->all_claimed()) {
					continue;
				}

				if (best == nullptr || r->priority > best->priority) {
					best = r;
				}
			}

			return best;
		}

		/*
			may_yield_to_urgent is true only for the pool workers. The thread that called
			process() must never drop out of its own region: it has nowhere else to go,
			and with an empty pool it would be the only one left to finish the work.
		*/
		void drain(region& r, const bool may_yield_to_urgent) {
			r.inside.fetch_add(1);

			/*
				Leaving has to happen even if a callback throws - a participant that
				vanished without decrementing would leave the region forever incomplete,
				and everyone waiting on it stuck.
			*/
			auto leave = augs::scope_guard([this, &r]() {
				/*
					Stop holding the others back the moment the last item is claimed,
					rather than when it finishes - otherwise they would trickle through
					one item per wake-up while this region drains its tail.
				*/
				if (r.priority == resource_priority::FIRST_FRAME && r.all_claimed()) {
					if (!r.urgency_cleared.exchange(true)) {
						urgent_pending.fetch_sub(1);
					}
				}

				{
					/*
						The lock is taken purely to order this decrement against the waiter
						in process(). Without it the decrement could land between that
						waiter's predicate check and its sleep, and the wake-up would be
						lost - which would hang the load until something else notified.
					*/
					auto lock = std::scoped_lock<std::mutex>(m);
					r.inside.fetch_sub(1);
				}

				region_finished.notify_all();
			});

			while (true) {
				const auto i = r.next.fetch_add(1);

				if (i >= r.count) {
					break;
				}

				try {
					r.callback(i);
				}
				catch (...) {
					auto lock = std::scoped_lock<std::mutex>(m);

					if (r.error == nullptr) {
						r.error = std::current_exception();
					}

					/* Claim the rest, so that the other participants wind down too. */
					r.next.store(r.count);
					break;
				}

				/*
					Choosing a region only on entry would not be enough: a worker that
					picked up the sounds would stay in them for a quarter of a second,
					while the atlas that appeared meanwhile waited on its own thread.
					Dropping out here lets it go and take the urgent one instead; the
					items left behind are still unclaimed, so nothing is lost.
				*/
				const bool yield_now =
					may_yield_to_urgent
					&& r.priority != resource_priority::FIRST_FRAME
					&& urgent_pending.load() > 0
				;

				if (yield_now) {
					break;
				}
			}
		}

		void quit_all_workers() {
			if (workers.empty()) {
				return;
			}

			{
				/*
					Set under the lock for the same reason as the decrement in drain():
					a worker that evaluated the wait predicate just before this would
					otherwise go to sleep past the notification, and join() below would
					never return.
				*/
				auto lock = std::scoped_lock<std::mutex>(m);
				shall_quit.store(true);
			}

			work_available.notify_all();

			for (auto& w : workers) {
				w.join();
			}

			workers.clear();
		}

	public:
		resource_workers() = default;

		~resource_workers() {
			quit_all_workers();
		}

		resource_workers(const resource_workers&) = delete;
		resource_workers& operator=(const resource_workers&) = delete;

		/*
			The count is frozen for the whole run: the pool is sized once, at startup,
			and the call is a no-op afterwards. Resizing later would have to tear down
			workers that may be mid-region, leaving it with nobody to finish it and
			whoever posted it stuck in process() - and a changed count cannot be tried
			out without a restart anyway.
		*/
		void start(const std::size_t num_workers) {
			if (!workers.empty()) {
				return;
			}

			for (std::size_t i = 0; i < num_workers; ++i) {
				workers.emplace_back([this]() {
					while (true) {
						region_ptr chosen;

						{
							auto lock = std::unique_lock<std::mutex>(m);

							work_available.wait(lock, [this, &chosen]() {
								if (shall_quit.load()) {
									return true;
								}

								chosen = find_unclaimed();
								return chosen != nullptr;
							});

							if (shall_quit.load()) {
								return;
							}
						}

						if (chosen != nullptr) {
							drain(*chosen, true);
						}
					}
				});
			}
		}

		std::size_t size() const {
			return workers.size();
		}

		template <class F>
		void process(const std::size_t n, F&& new_callback, const resource_priority priority = resource_priority::NORMAL) {
			if (n == 0) {
				return;
			}

			auto r = std::make_shared<region>();
			r->callback = std::forward<F>(new_callback);
			r->count = n;
			r->priority = priority;

			const bool is_urgent = priority == resource_priority::FIRST_FRAME;

			{
				auto lock = std::scoped_lock<std::mutex>(m);

				if (is_urgent) {
					urgent_pending.fetch_add(1);
				}

				regions.emplace_back(r);
			}

			work_available.notify_all();

			drain(*r, false);

			auto lock = std::unique_lock<std::mutex>(m);
			region_finished.wait(lock, [&r]() { return r->complete(); });

			regions.erase(std::remove(regions.begin(), regions.end(), r), regions.end());

			if (r->error != nullptr) {
				std::rethrow_exception(r->error);
			}
		}
	};
#endif

	/*
		The single pool shared by every content loading path, so that the atlas bake
		and the sound loading draw from one budget instead of two. Sized once at
		startup - a change to the setting takes effect on the next launch, because
		resizing while a load is in flight would pull the workers out from under it.

		A namespace-scope variable rather than a function-local static on purpose: the
		project builds with -fno-threadsafe-statics (CMakeLists.txt), so a local static
		would have no guard, and this is reached from several threads.
	*/
	inline resource_workers shared_resource_workers;

	inline resource_workers& get_resource_workers() {
		return shared_resource_workers;
	}
}
