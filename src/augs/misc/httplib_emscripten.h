#pragma once
#include <emscripten/fetch.h>
#include <string>
#include <iostream>
#include <memory>

namespace augs {
	class emscripten_http {
		std::string base_url_;
		int io_timeout = 5;

	public:
		struct response {
			int status = -1;
			std::string body;
		};

		emscripten_http(const std::string& base_url, const int io_timeout) : base_url_(base_url), io_timeout(io_timeout) {}

		using result = std::unique_ptr<response>;

		template <class... T>
		result Post(T...) {
			return nullptr;
		}

		auto Get(const std::string& location, std::function<void(std::size_t, std::size_t)> on_progress = nullptr) {
			(void)on_progress;

			emscripten_fetch_attr_t attr;
			emscripten_fetch_attr_init(&attr);

			strcpy(attr.requestMethod, "GET");
			attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
			attr.timeoutMSecs = io_timeout * 1000;

			std::string full_url = base_url_ + location;
			emscripten_fetch_t* fetch = emscripten_fetch(&attr, full_url.c_str());

			response resp;
			resp.status = fetch->status;

			if (fetch->status == 200) {
				resp.body.assign(reinterpret_cast<const char*>(fetch->data), fetch->numBytes);
			}

			emscripten_fetch_close(fetch);
			return std::make_unique<response>(std::move(resp));
		}

		void set_keep_alive(bool) {
		
		}
	};
}
