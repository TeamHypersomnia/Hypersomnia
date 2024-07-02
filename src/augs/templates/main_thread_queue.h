#if PLATFORM_WEB
#include <iostream>
#include <thread>
#include "augs/misc/mutex.h"
#include <queue>
#include <functional>
#include <future>
#include <optional>

class main_thread_queue {
public:
    static main_thread_queue& get_instance() {
        static main_thread_queue instance;
        return instance;
    }

	void save_main_thread_id() {
		main_thread_id = std::this_thread::get_id();
	}

    template<typename Func>
    static decltype(auto) execute(Func&& func) {
		if (std::this_thread::get_id() == get_instance().main_thread_id) {
            return func();
        }
		else {
			return get_instance().execute_impl(std::forward<Func>(func));
        }
    }

    template<typename Func>
    static void execute_async(Func&& func) {
		if (std::this_thread::get_id() == get_instance().main_thread_id) {
            func();
        }
		else {
			get_instance().enqueue_task(std::forward<Func>(func));
        }
    }

    void process_tasks() {
        std::queue<std::function<void()>> localTasks;

        // Lock and copy tasks to local queue, then unlock
        {
            augs::scoped_lock lock(queueMutex);
            if (tasks.empty()) {
                return;  // No tasks to execute, return immediately
            }
            std::swap(tasks, localTasks);
        }

        // Process all tasks in the local queue without holding the mutex
        while (!localTasks.empty()) {
            auto task = std::move(localTasks.front());
            localTasks.pop();
            task();  // Execute the task
        }
    }

private:
	std::optional<std::thread::id> main_thread_id;
    augs::mutex queueMutex;
    std::queue<std::function<void()>> tasks;

    template<typename Func>
    decltype(auto) execute_impl(Func&& func) {
        using ReturnType = std::invoke_result_t<Func>;

        std::promise<ReturnType> promise;
        auto future = promise.get_future();

        enqueue_task([&func, &promise]() {
            try {
                if constexpr (std::is_void_v<ReturnType>) {
                    func();
                    promise.set_value();
                } else {
                    promise.set_value(func());
                }
            } catch (...) {
                promise.set_exception(std::current_exception());
            }
        });

        return future.get();
    }

    template<typename Func>
    void enqueue_task(Func&& func) {
        augs::scoped_lock lock(queueMutex);
        tasks.emplace(std::forward<Func>(func));
    }
};
#else
class main_thread_queue {
public:
    template<typename Func>
    static decltype(auto) execute(Func&& func) {
        return func();
    }

    template<typename Func>
    static void execute_async(Func&& func) {
        func();
    }
};
#endif
