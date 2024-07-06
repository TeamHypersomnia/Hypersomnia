#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <future>
#include <atomic>
#include <memory>

#include "augs/templates/thread_templates.h"

namespace augs {
    struct http_response {
        int status = -1;
        std::string body;
    };

	using async_get_progress_callback = std::function<bool(std::size_t, std::size_t)>;

	class async_response;
	using async_response_ptr = std::shared_ptr<async_response>;

    class async_response {
        std::atomic<bool> ready;
        http_response resp;
        async_get_progress_callback on_progress;

		http_response get() {
			return std::move(resp);
		}

    public:
		std::string user_data;

		friend http_response get_once(async_response_ptr& p);

        async_response() : ready(false), resp{-1, ""} {}

        bool is_ready() const {
            return ready.load();
        }

        bool in_progress() const {
            return !is_ready();
        }

        void set(const http_response& resp_data) {
            resp = resp_data;
            ready.store(true);
        }

        void set_on_progress(const async_get_progress_callback& callback) {
            on_progress = callback;
        }

        const async_get_progress_callback& get_on_progress() const {
            return on_progress;
        }
    };

    inline bool is_ready(const async_response_ptr& p) {
        return p && p->is_ready();
    }

    inline bool in_progress(const async_response_ptr& p) {
        return p && !p->is_ready();
    }

    inline http_response get_once(async_response_ptr& p) {
        auto r = p->get();
        p = nullptr;
        return r;
    }
}
